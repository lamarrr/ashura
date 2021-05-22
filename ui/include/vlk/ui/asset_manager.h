#pragma once

#include <atomic>
#include <cinttypes>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include "stx/result.h"
#include "vlk/ui/primitives.h"
#include "vlk/ui/raster_context.h"
#include "vlk/utils/utils.h"

// NOLINTFILE(runtime/references)
namespace vlk {
namespace ui {

/*
struct CancToken{
alignas(std::hardware_destructive_interference_size) std::atomic<Token> token;
};
*/

struct Asset {
 public:
  Asset() : size_bytes_{0} {}
  virtual ~Asset() {}

  uint64_t size_bytes() const { return size_bytes_; }

 protected:
  void update_size(uint64_t size) { size_bytes_ = size; }

 private:
  uint64_t size_bytes_ = 0;
};

struct AssetLoadArgs {
  AssetLoadArgs() {}
  virtual ~AssetLoadArgs() {}
};

// loaders can be shared accross multiple threads and thus share the same memory
// space. therefore, `load()` is made const to prevent modifying the address
// space across threads (data races).
struct AssetLoader {
  AssetLoader() {}
  virtual ~AssetLoader() {}

  // must be thread-safe
  virtual std::unique_ptr<Asset> load(RasterContext const &,
                                      AssetLoadArgs const &) const {
    return std::make_unique<Asset>();
  }
};

// Asset Manager Requirements
//
// - we want to be able to load by tag, tags must be unique
// - we want to be able to view usage, drop, reload and hit statistics
// - we want to be able to drop the items when not in use or whenever we want or
// feel like
// - we want persistence of the assets in certain cases, i.e. icons that are
// certain to be used in a lot of places and are cheap to have in memory
// - we want to provide asynchronous data loading without blocking the main
// thread.
// - we want to be able to relay the status of the loaded assets
//

enum class AssetState : uint8_t { Loading, Loaded, Unloaded };

enum class AssetError : uint8_t { TagExists, InvalidTag, IsLoading };

struct AssetManager {
 public:
  AssetManager(RasterContext const &context)
      : data_{},
        submission_queue_{},
        submission_queue_mutex_{},
        completion_queue_{},
        completion_queue_mutex_{},
        cancelation_token_{std::make_shared<AtomicToken>(Token::Running)},
        context_{&context},
        worker_thread_{worker_thread_task,
                       std::ref(submission_queue_),
                       std::ref(submission_queue_mutex_),
                       std::ref(completion_queue_),
                       std::ref(completion_queue_mutex_),
                       cancelation_token_,
                       std::ref(*context_)} {}

  //! `requires_persistence`: some data assets must just persist. i.e. icons and
  //! frequently used data. internet-loaded data file-loaded data should not
  //! necessarily persist
  //!
  //! non-persistent ones will be discarded/unloaded after a period of time
  //!
  //!
  auto add(std::string_view tag,
           std::unique_ptr<AssetLoadArgs const> &&load_args,
           std::shared_ptr<AssetLoader const> &&loader,
           bool requires_persistence = false)
      -> stx::Result<stx::NoneType, AssetError> {
    VLK_ENSURE(load_args != nullptr);
    VLK_ENSURE(loader != nullptr);

    // references are not invalidated
    auto const [iterator, was_inserted] = data_.try_emplace(
        std::string{tag},
        AssetData{std::move(load_args), std::move(loader), requires_persistence,
                  stx::None, AssetState::Unloaded, Ticks{}, false});

    if (!was_inserted) {
      return stx::Err(AssetError::TagExists);
    }

    return stx::Ok<stx::NoneType>({});
  }

  // if asset has an entry and it has been discarded, we need to now create make
  // a reload and then return that the asset is being loaded back
  auto get(std::string const &tag)
      -> stx::Result<std::shared_ptr<Asset>, AssetError> {
    auto const pos = data_.find(tag);
    if (pos == data_.end()) return stx::Err(AssetError::InvalidTag);

    switch (pos->second.state) {
      case AssetState::Loaded:
        pos->second.just_accessed = true;
        return stx::Ok(pos->second.asset.clone().unwrap());
      case AssetState::Loading:
        return stx::Err(AssetError::IsLoading);
      case AssetState::Unloaded:
        pos->second.state = AssetState::Loading;
        // this means data must not be removed from the data map whilst
        // reference is in the submission queue since we are holding a reference
        // to it
        submit_task(SubmissionData{pos->first, pos->second.loader.get(),
                                   pos->second.load_args.get()});

        VLK_LOG("Submitted asset `{}` to worker thread for loading",
                pos->first);

        return stx::Err(AssetError::IsLoading);
    }
  }

  // TODO(lamarrr): allow suggesting removal if the person that added the asset
  // is cetain that the asset wont be used again. this will enusre that the
  // asset is removed before the max tick provided

  void tick(std::chrono::nanoseconds interval) {
    bool size_changed = false;
    {
      std::lock_guard guard{completion_queue_mutex_};

      for (size_t i = 0; i < completion_queue_.size(); i++) {
        CompletionData cmpl_data = std::move(completion_queue_.front());
        auto const iter = data_.find(std::string(cmpl_data.tag));
        VLK_ENSURE(iter != data_.end());
        iter->second.state = AssetState::Loaded;
        iter->second.asset =
            stx::Some(std::shared_ptr{std::move(cmpl_data.asset)});

        VLK_LOG("Loaded asset with tag `{}` of size: {} bytes", iter->first,
                iter->second.asset.value()->size_bytes());

        size_changed = true;
      }

      // a poor man's std::queue::clear
      completion_queue_ = {};
    }

    for (auto &[tag, entry] : data_) {
      if (entry.just_accessed) {
        entry.stale_ticks.reset();
      } else if (entry.state == AssetState::Loaded) {
        entry.stale_ticks++;
      }

      entry.just_accessed = false;

      if (!entry.requires_persistence && entry.stale_ticks > max_stale_ticks_ &&
          entry.state == AssetState::Loaded &&
          entry.asset.value().use_count() == 1) {
        VLK_LOG(
            "Asset with tag `{}` and size {} bytes has been stale and not in "
            "use "
            "for {} ticks. "
            "Asset will be "
            "discarded",
            tag, entry.asset.value()->size_bytes(), entry.stale_ticks.count());
        entry.asset = stx::None;
        entry.state = AssetState::Unloaded;

        size_changed = true;
      }
    }

    if (size_changed) {
      uint64_t total_size = 0;
      for (auto &[tag, entry] : data_) {
        if (entry.state == AssetState::Loaded) {
          total_size += entry.asset.value()->size_bytes();
        }
      }
      VLK_LOG("Present total assets size: {} bytes", total_size);
    }
  }

  ~AssetManager() { shutdown_worker_thread(); }

 private:
  enum class Token : uint8_t { Running = 0, Cancel, Exited };

  using AtomicToken = std::atomic<Token>;

  using CancelationToken = std::shared_ptr<AtomicToken>;

  // NOTE: only the pointer value of load_args and loader are shared with the
  // worker threads. this prevents data races even along struct members.
  struct AssetData {
    std::unique_ptr<AssetLoadArgs const> load_args;
    std::shared_ptr<AssetLoader const> loader;
    bool requires_persistence = false;
    // TODO(lamarrr): should asset and image_asset be const?
    stx::Option<std::shared_ptr<Asset>> asset;
    AssetState state = AssetState::Loading;
    Ticks stale_ticks = Ticks{0};
    bool just_accessed = false;
  };

  struct SubmissionData {
    std::string tag;
    AssetLoader const *loader = nullptr;
    AssetLoadArgs const *load_args = nullptr;
  };

  struct CompletionData {
    std::string tag;
    std::unique_ptr<Asset> asset;
  };

  void submit_task(SubmissionData &&subm_data) {
    std::lock_guard guard{submission_queue_mutex_};
    submission_queue_.push(std::move(subm_data));
  }

  static void backoff_spin_delay(uint64_t iteration) {
    if (iteration < 64) {
      // immediate spinning
    } else if (iteration < 128) {
      std::this_thread::yield();
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(125));
    }
  }

  // is RasterContext safe across threads?
  //
  // the worker threads only read the submission data and not modify them
  //
  //
  static void worker_thread_task(std::queue<SubmissionData> &submission_queue,
                                 std::mutex &submission_queue_mutex,
                                 std::queue<CompletionData> &completion_queue,
                                 std::mutex &completion_queue_mutex,
                                 CancelationToken cancelation_token,
                                 RasterContext const &context) {
    do {
      bool gotten_task = false;

      // it doesn't make sense to lock the whole queue at once and then start
      // executing the task because the asset load operation could be
      // time-consuming. we thus lock and unlock immediately after popping a
      // task from the queue so that more tasks can be added while we are
      // processing one.

      for (uint64_t i = 0;
           !gotten_task && cancelation_token->load() != Token::Cancel; i++) {
        submission_queue_mutex.lock();

        if (submission_queue.size() != 0) {
          SubmissionData task = std::move(submission_queue.front());
          submission_queue.pop();
          submission_queue_mutex.unlock();
          gotten_task = true;

          CompletionData cmpl_data{std::move(task.tag),
                                   task.loader->load(context, *task.load_args)};

          {
            std::lock_guard guard{completion_queue_mutex};
            completion_queue.push(std::move(cmpl_data));
          }
        } else {
          submission_queue_mutex.unlock();
          backoff_spin_delay(i);
        }
      }
    } while (cancelation_token->load() != Token::Cancel);

    VLK_LOG("Asset manager worker thread exiting...");
    cancelation_token->store(Token::Exited);
  }

  void shutdown_worker_thread() {
    cancelation_token_->store(Token::Cancel);
    for (uint64_t i = 0; cancelation_token_->load() != Token::Exited; i++) {
      backoff_spin_delay(i);
    }

    worker_thread_.join();
    VLK_LOG("Asset manager worker thread shut down");
  }

  std::map<std::string, AssetData> data_;

  std::queue<SubmissionData> submission_queue_;
  std::mutex submission_queue_mutex_;

  std::queue<CompletionData> completion_queue_;
  std::mutex completion_queue_mutex_;

  CancelationToken cancelation_token_;

  RasterContext const *context_ = nullptr;

  std::thread worker_thread_;

  Ticks max_stale_ticks_ = Ticks{100};
};

}  // namespace ui
}  // namespace vlk
