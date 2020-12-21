#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstring>
#include <map>
#include <utility>
#include <vector>

#include "stx/option.h"
#include "vlk/gl.h"
#include "vlk/utils.h"
#include "vulkan/vulkan.h"

namespace vlk {

struct MemoryCommit {
  // offset into the device memory
  uint64_t offset;
  // true size of the memory commit (can be larger than requested size)
  uint64_t size;
  // the device memory
  VkDeviceMemory memory;
};

// not thread-safe
// allocates memory in blocks and frees all of the blocks at the end of it's
// lifetime (when destroy is called)
struct BlockAllocator {
  struct MemoryBlock {
    struct Partition {
      uint64_t offset;
      uint64_t size;
      bool in_use;

     private:
      MemoryCommit commit_with(VkDeviceMemory memory) noexcept {
        this->in_use = true;
        MemoryCommit commit{};
        commit.memory = memory;
        commit.offset = offset;
        commit.size = size;
        return commit;
      }

      void uncommit() noexcept { this->in_use = false; }

      friend struct MemoryBlock;
    };

   private:
    std::vector<Partition> partitions_;
    VkDeviceMemory memory_;

    stx::Option<MemoryMap> memory_map_;
    uint64_t size_;

    // if there are any inactive usable partitions within the blocks and try to
    // use them first, else
    stx::Option<MemoryCommit> try_commit(size_t bytes) noexcept {
      // check existing partitions
      for (auto& partition : partitions_) {
        if (!partition.in_use && partition.size >= bytes) {
          return stx::Some(partition.commit_with(memory_));
        }
      }

      Partition last_partition =
          partitions_.empty() ? Partition{} : partitions_.back();

      // if all partitions have exhausted the memory block
      if (last_partition.offset + last_partition.size + bytes > size_)
        return stx::None;

      // create new partition
      partitions_.push_back({});
      auto& partition = partitions_.back();
      partition.offset = partitions_.size() > 0
                             ? last_partition.offset + last_partition.size
                             : 0;
      partition.size = bytes;

      return stx::Some(partition.commit_with(memory_));
    }

    void uncommit(size_t offset) noexcept {
      auto partition = std::find_if(partitions_.begin(), partitions_.end(),
                                    [offset](Partition const& partition) {
                                      return partition.offset == offset;
                                    });
      VLK_ENSURE(partition != partitions_.end(),
                 "Attempting to release unallocated partition");
      partition->uncommit();
    }

    // user is expected to already check offset and size
    MemoryMap get_submap(VkDevice device, uint64_t offset, uint64_t size) {
      VLK_ENSURE(offset + size <= size_,
                 "Requested memory map outside of memory range");
      memory_map_ = stx::Some(memory_map_.clone().unwrap_or_else([=]() {
        auto map = map_memory(device, memory_, 0, size_);
        return map;
      }));

      return memory_map_.clone()
          .map([=](MemoryMap map) {
            return MemoryMap{offset, stx::Span<uint8_t volatile>(
                                         map.span.data() + offset, size)};
          })
          .unwrap();
    }

    void unmap(VkDevice device) {
      vkUnmapMemory(device, memory_);
      memory_map_ = stx::None;
    }

    friend struct BlockAllocator;
  };

  stx::Option<stx::Ref<MemoryBlock>> add_memory_block(
      VkDevice device) noexcept {
    // create new memory block
    if (memory_blocks_.size() >= max_allocations_count_) return stx::None;

    // create new memory block
    VkMemoryAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = bytes_per_block_;
    allocate_info.memoryTypeIndex = memory_type_index_;

    VkDeviceMemory memory;
    VLK_MUST_SUCCEED(vkAllocateMemory(device, &allocate_info, nullptr, &memory),
                     "Unable to allocate memory for memory block");

    MemoryBlock block{};
    block.memory_ = memory;
    block.size_ = bytes_per_block_;

    memory_blocks_.push_back(std::move(block));

    return stx::some_ref(memory_blocks_.back());
  }

  MemoryMap get_memory_submap(VkDevice device, VkDeviceMemory memory,
                              uint64_t offset, uint64_t size) {
    auto pos =
        std::find_if(memory_blocks_.begin(), memory_blocks_.end(),
                     [=](auto const& par) { return par.memory_ == memory; });
    VLK_ENSURE(pos != memory_blocks_.end(),
               "Requesting for memory map for memory not allocated from this "
               "allocator");
    return pos->get_submap(device, offset, size);
  }

  void unmap_memory(VkDevice device, VkDeviceMemory memory) {
    auto pos =
        std::find_if(memory_blocks_.begin(), memory_blocks_.end(),
                     [=](auto const& par) { return par.memory_ == memory; });
    VLK_ENSURE(pos != memory_blocks_.end(),
               "Requesting for memory map for memory not allocated from this "
               "allocator");
    pos->unmap(device);
  }

  BlockAllocator() = default;
  BlockAllocator(BlockAllocator const&) = delete;
  BlockAllocator& operator=(BlockAllocator const&) = delete;
  BlockAllocator(BlockAllocator&& other)
      : memory_type_index_{other.memory_type_index_},
        memory_blocks_{std::move(other.memory_blocks_)},
        bytes_per_block_{other.bytes_per_block_},
        max_allocations_count_{other.max_allocations_count_} {}
  BlockAllocator& operator=(BlockAllocator&& other) {
    memory_type_index_ = other.memory_type_index_;
    memory_blocks_ = std::move(other.memory_blocks_);
    bytes_per_block_ = other.bytes_per_block_;
    max_allocations_count_ = other.max_allocations_count_;
    return *this;
  }

  static BlockAllocator create(uint32_t memory_type_index,
                               size_t max_allocations_count,
                               size_t bytes_per_block) noexcept {
    auto allocator = BlockAllocator{};
    VLK_ENSURE(max_allocations_count > 0, "maximum allocations count is 0");
    VLK_ENSURE(bytes_per_block > 0, "bytes per block is 0");
    allocator.memory_type_index_ = memory_type_index;
    allocator.max_allocations_count_ = max_allocations_count;
    allocator.bytes_per_block_ = bytes_per_block;

    return allocator;
  }

  // allocates at-least {bytes} bytes of memory
  stx::Option<MemoryCommit> allocate(VkDevice device, uint64_t bytes) noexcept {
    VLK_ENSURE(
        bytes <= bytes_per_block_,
        "Requested bytes size exceeds max requestable device memory size");

    for (auto& block : memory_blocks_) {
      auto commit = block.try_commit(bytes);
      if (commit.is_some()) return commit;
    }

    MemoryBlock& block = add_memory_block(device).expect(
        "Unable to create new memory block, allocation count limit reached");

    MemoryCommit commit =
        block.try_commit(bytes).expect("Unexpected memory block state");

    return stx::Some(std::move(commit));
  }

  void deallocate(MemoryCommit const& commit) noexcept {
    auto block = std::find_if(memory_blocks_.begin(), memory_blocks_.end(),
                              [&commit](MemoryBlock const& block) {
                                return block.memory_ == commit.memory;
                              });

    VLK_ENSURE(block != memory_blocks_.end(),
               "Attempted to release unallocated device memory");

    block->uncommit(commit.offset);
  }

  void destroy(VkDevice device) noexcept {
    for (auto const& block : memory_blocks_) {
      for (auto const& partition : block.partitions_) {
        VLK_ENSURE(!partition.in_use,
                   "Committed memory in use and not deallocated");
      }

      vkFreeMemory(device, block.memory_, nullptr);
    }
  }

 private:
  uint32_t memory_type_index_;
  std::vector<MemoryBlock> memory_blocks_;
  uint64_t bytes_per_block_;
  size_t max_allocations_count_;
};

struct AllocationMonitor {
  std::atomic<size_t> allocations_;
  size_t max_allocations_;
  constexpr auto max_allocations() const { return max_allocations_; }
  auto current_allocation_count() const { return allocations_.load(); }
};

// active allocators and properties
template <typename AllocatorT = BlockAllocator>
struct AllocatorRegistry {
 public:
 private:
  // heap index, allocator
  std::map<size_t, AllocatorT> allocators_;
  AllocationMonitor& monitor;

  AllocatorT get_allocator();
};

// TODO(lamarrr): change block-size to min_block_size
template <VkBufferUsageFlagBits Usage, VkSharingMode SharingMode,
          VkMemoryPropertyFlagBits MemoryProperties>
struct Buffer {
  static constexpr auto usage = Usage;
  static constexpr auto sharing_mode = SharingMode;
  static constexpr auto memory_properties = MemoryProperties;

  // TODO(lamarrr): let it receive a span of allocators and decide whether to
  // use any of them or create a new one
  static Buffer create(VkDevice device, VkPhysicalDevice physical_device,
                       uint64_t bytes_size,
                       uint64_t block_size) {  // NOLINT
    auto buffer = create_buffer(device, bytes_size, usage, sharing_mode);
    auto requirements = get_memory_requirements(device, buffer);

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physical_device, &properties);

    auto memory_type =
        find_suitable_memory_type(physical_device, requirements,
                                  memory_properties)
            .expect("Could not find suitable memory type for buffer");

    auto allocator = BlockAllocator::create(
        memory_type, properties.limits.maxMemoryAllocationCount, block_size);

    auto commit = allocator.allocate(device, bytes_size)
                      .expect("unable to allocate memory for buffer");

    bind_memory_to_buffer(device, buffer, commit.memory);
    return Buffer{buffer, std::move(commit), std::move(allocator)};
  }

  void destroy(VkDevice device) {
    allocator_.deallocate(commit_);
    allocator_.destroy(device);
    vkDestroyBuffer(device, buffer_, nullptr);
  }

  // TODO(lamarrr): we are using a partition of the memory, a memory map may
  // already exist, if we have two buffers using the same memory then we can't
  // write to them at the same time we can alternatively always have a memory
  // map for the whole memory region and then try to get that.
  // - one map at a time
  // problem is that multiple buffers using the same memory can't be used in a
  // multi-threaded nature
  // offset repressents offset into this buffer
  void write(VkDevice device, uint64_t offset,
             stx::Span<uint8_t const> const& data) {
    VLK_ENSURE(offset + data.size() <= size());
    VLK_ENSURE(static_cast<bool>(memory_properties &
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));

    auto buffer_map = allocator_.get_memory_submap(device, commit_.memory,
                                                   commit_.offset, size());

    std::copy(data.begin(), data.end(), buffer_map.span.begin() + offset);

    allocator_.unmap_memory(device, commit_.memory);

    // wirtes may not immediately take effect, user might need to flush
  }

  uint64_t size() { return size_; }

  VkBuffer buffer_;
  uint64_t size_;
  MemoryCommit commit_;
  BlockAllocator allocator_;
};

};  // namespace vlk