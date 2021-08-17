#include <atomic>
#include <cinttypes>
#include <cstddef>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

namespace rc {

// the table is also free to delete itself once weak_ref and strong_ref reach 0.
// i.e. intrusive/self-managed lifetime if you may. it can also delegate the
// destruction of itself and its associated resource. i.e. a memory pool or
// bulk-allocated memory.
//
//
//
// this enables a couple of use-cases. i.e. use in embedded systems and
// single-threaded environments, use in scenarios where the user is certain the
// handle or resource will outlive the ArcResource and custom memory management
// solutions (i.e. pool-based solutions).
//
// this is a scalable abstraction over resource management, ***I think***.
//
// coincidentally, this should be able to support constexpr reference counting
// by swapping out the `RefTableType` arguments of the Rc and WkResource
//
//
// this tries to decouple management of the resource from the resource itself.
//
// resources can be any type. not just pointers which is only what shared_ptr
// supports.
//
//
//
// I often find myself needing to group objects and avoiding needless
// allocations when I'm certain the resource will both be valid.
//
//
// i.e. a curl http client:
//
// struct ClientHandle{
//  CURL * easy;
//  CURLM * multi;
// };
//
//
// easy depends on multi, and I intend to have their lifetimes bounded together.
// I can't reasonably do this using shared_ptr since I'd have two allocations
// for two control blocks plus one ref-count plus one handle struct allocation
// .i.e.
//
// shared_ptr<CURLM> multi{ curl_multi_init(), CurlMultiHandleDeleter{}  }; //
// allocates a control block struct
//
// CurlEasyHandle{
//  CURL * easy;
//  shared_ptr<CURLM> multi;
// };
//
// shared_ptr<CurlEasyHandle> easy { new CurlEasyHandle{curl_easy_init(),
//              multi}, CurlEasyHandle::Deleter{}  };  // allocates memory for
//              the handle
// struct and allocates another control block + intentional ref-count
//
//
//
//
//

// the table handle should not be valid once the weak and strong refs are both
// 0.
//
// the operations are specified atomically so they can be used for reftable
// implementations that choose to use atomic (multi-threaded) or non-atomic
// operations.
//
struct VirtualRefTableHandle {
  // increase the strong ref count of the associated resource.
  // resource must always be valid with a >0 strong ref count.
  //
  virtual void strong_ref() = 0;
  // increase the weak ref count of the associated resource.
  // resource need not be valid for a >0 weak ref count. but this handle must be
  // valid if either strong ref count or weak ref count are >0.
  //
  // this handle need not be valid after weak ref count and strong ref count
  // become 0
  virtual void weak_ref() = 0;
  // reduce the strong ref count
  virtual void strong_deref() = 0;
  // reduce the weak ref count
  virtual void weak_deref() = 0;
  // try to upgrade a weak ref to a strong ref. if the strong ref count is
  // > 0, the associated resource must be available and the upgrade must
  // succeed.
  virtual bool try_weak_upgrade() = 0;
};

namespace impl {

struct StaticStorageVirtualRefTableHandle final : public VirtualRefTableHandle {
  virtual void strong_ref() override final {}
  virtual void weak_ref() override final {}
  virtual void strong_deref() override final {}
  virtual void weak_deref() override final {}
  virtual bool try_weak_upgrade() override final { return true; }
};

struct NoopVirtualRefTableHandle final : public VirtualRefTableHandle {
  virtual void strong_ref() override final {}
  virtual void weak_ref() override final {}
  virtual void strong_deref() override final {}
  virtual void weak_deref() override final {}
  virtual bool try_weak_upgrade() override final { return true; }
};

struct RefCountBase {};

template <typename ValueType>
struct Manager final : public VirtualRefTableHandle {
  ValueType* value = nullptr;
  alignas(alignof(std::max_align_t) *
          2) std::atomic<uint64_t> strong_ref_count = 0;
  alignas(alignof(std::max_align_t) *
          2) std::atomic<uint64_t> weak_ref_count = 0;

  template <typename... Args>
  Manager* create(Args&&... args) {
    return new Manager{new ValueType{std::forward<Args>(args)...}};
  }

  virtual void strong_ref() override final {
    strong->count.fetch_add(1, std::memory_order_relaxed);
  }

  virtual void weak_ref() override final {
    weak_ref_count.fetch_add(1, std::memory_order_relaxed);
  }

  virtual void strong_deref() override final {
    if (strong->count.fetch_sub(1, std::memory_order_relaxed) == 0) {
      delete value;
    }
  }

  virtual void weak_deref() override final {
    if (weak_ref_count.fetch_sub(1, std::memory_order_relaxed) == 0) {
      delete this;
    }
  }

  virtual bool try_weak_upgrade() override final {
    // relies on wrapping since we are using uint64
    if (strong->count.fetch_add(1, std::memory_order_relaxed) == 1) {
      return false;
    } else {
      return true;
    }
  }
};

static StaticStorageVirtualRefTableHandle
    static_storage_virtual_ref_table_handle;
// we used this to help prevent cases where we have to perform branches to check
// validity of the table
static NoopVirtualRefTableHandle noop_virtual_ref_table_handle;

}  // namespace impl

// VirtualRefTable must be outlived by VirtualRefTableHandle
struct VirtualRefTable {
  // a static reftable handle represents a no-op. i.e. no operation is required
  // for managing lifetimes of the handle.
  static VirtualRefTable make_static() {
    return VirtualRefTable{impl::static_storage_virtual_ref_table_handle};
  }

  explicit VirtualRefTable(VirtualRefTableHandle& handle) : handle_{&handle} {}

  // default-initialized with a no-op.
  // this will not cause a fatal crash
  VirtualRefTable() : handle_{&impl::noop_virtual_ref_table_handle} {}

  // on-copy, the reference tables must refer to the same handle
  VirtualRefTable(VirtualRefTable const& other) = default;
  VirtualRefTable& operator=(VirtualRefTable const& other) = default;

  // on-move, the reference table must copy and then invalidate the other
  // refrence table's handle, the moved-from table is required to be valid but
  // unable to affect the associated state of the resource, i.e. (no-op). why
  // not nullptr? nullptr means we'd have to perform a check everytime we want
  // to send calls to the reftable (weak_ref, strong_ref, weak_deref,
  // strong_deref, try_weak_upgrade). but with a no-op we don't have any
  // branches, though we'd have an extra copy on move (new noop reftable pointer
  // assignment).
  //
  VirtualRefTable(VirtualRefTable&& other) : handle_{other.handle_} {
    // unarm other and prevent it from affecting the state of any object
    other.handle_ = &impl::noop_virtual_ref_table_handle;
  }

  VirtualRefTable& operator=(VirtualRefTable&& other) {
    // unarm other and prevent it from affecting the state of any object
    handle_ = other.handle_;
    other.handle_ = &impl::noop_virtual_ref_table_handle;
    return *this;
  }

  void strong_ref() const { handle_->strong_ref(); }

  void weak_ref() const { handle_->weak_ref(); }

  void strong_deref() const { handle_->strong_deref(); }

  void weak_deref() const { handle_->weak_deref(); }

  bool try_weak_upgrade() const { return handle_->try_weak_upgrade(); }

 private:
  VirtualRefTableHandle* handle_;
};

template <typename T>
constexpr bool is_resource_handle_type = std::is_default_constructible_v<T>&&
    std::is_copy_constructible_v<T>&& std::is_move_constructible_v<T>&&
        std::is_copy_assignable_v<T>&& std::is_move_assignable_v<T>;

// TODO(lamarrr)
template <typename T>
constexpr bool is_ref_table_type = true;

// recommended that HandleType be trivial
//
// Handle types are just values to be passed and moved around and whose validity
// is guaranteed by the RefTableType
//
// we do require that the ref_table_ be valid and become no-op when moved-from
// and no-op when default-constructed.
//
//
template <typename HandleType, typename RefTableType = VirtualRefTable>
struct Rc {
  template <typename WeakRcHandleType, typename WeakRcRefTableType>
  friend struct WeakRc;

  static_assert(is_resource_handle_type<HandleType>);
  static_assert(is_ref_table_type<RefTableType>);

  // a default-constructed handle and ref_table_. not necessarily invalid.
  // the invalidity of the handle is determined by the semantics of the handle
  // type itself. i.e. is the handle type valid when default-constructed?
  // it doesn't affect us since we don't use the handle type directly in the
  // struct.
  //
  constexpr Rc() : handle_{}, ref_table_{} {}

  // take strong ownership of a yet-to-be strong-ref'd handle
  static constexpr Rc adopt(HandleType&& handle, RefTableType&& table) {
    return Rc{std::move(handle), std::move(table)};
  }

  // take strong ownership of an already strong-ref'd handle
  static constexpr Rc share(HandleType&& handle, RefTableType&& table) {
    table.strong_ref();
    return adopt(std::move(handle), std::move(table));
  }

  // adopt a handle that is guaranteed to be valid for the lifetime of this Rc
  // struct and any Rc or Weak structs constructed or assigned from it.
  // typically used for static storage lifetimes.
  //
  // it is advised that this should not be used for scope-local storage as it
  // would be difficult to guarantee that a called function does not retain a
  // copy or move an Rc constructed using this method.
  //
  static constexpr Rc adopt_static(HandleType&& handle) {
    return adopt(std::move(handle), RefTableType::make_static());
  }

  // share
  constexpr Rc(Rc const& other)
      : handle_{other.handle_}, ref_table_{other.ref_table_} {
    other.ref_table_.strong_ref();
  }

  // adopt
  constexpr Rc(Rc&& other)
      : handle_{std::move(other.handle_)},
        ref_table_{std::move(other.ref_table_)} {}

  // share
  constexpr Rc& operator=(Rc const& other) {
    other.ref_table_.strong_ref();
    handle_ = other.handle;
    ref_table_ = other.ref_table_;
    return *this;
  }

  // adopt
  constexpr Rc& operator=(Rc&& other) {
    handle_ = std::move(other.handle_);
    ref_table_ = std::move(other.ref_table_);
    return *this;
  }

  constexpr HandleType get() const { return handle_; }

  // strong release
  ~Rc() { ref_table_.strong_deref(); }

 private:
  HandleType handle_;
  RefTableType ref_table_;

  constexpr Rc(HandleType&& handle, RefTableType&& ref_table)
      : handle_{std::move(handle)}, ref_table_{std::move(ref_table)} {}
};

template <typename HandleType, typename RefTableType = VirtualRefTable>
struct WeakRc {
  static_assert(is_resource_handle_type<HandleType>);
  static_assert(is_ref_table_type<RefTableType>);

  constexpr WeakRc() : handle_{}, ref_table_{RefTableType::make_static()} {}

  // take weak ownership of a yet-to-be weak-ref'd handle
  static constexpr WeakRc adopt(HandleType&& handle, RefTableType&& table) {
    return Rc{std::move(handle), std::move(table)};
  }

  // take weak ownership of an already weak-ref'd handle
  static constexpr WeakRc share(HandleType&& handle, RefTableType&& table) {
    table.weak_ref();
    return adopt(std::move(handle), std::move(table));
  }

  explicit constexpr WeakRc(Rc<HandleType, RefTableType> const& arc)
      : handle_{arc.handle_}, ref_table_{arc.ref_table_} {
    // share
    arc.ref_table_.weak_ref();
  }

  constexpr WeakRc(WeakRc const& other)
      : handle_{other.handle_}, ref_table_{other.ref_table_} {
    // share
    other.ref_table_.weak_ref();
  }

  constexpr WeakRc(WeakRc&& other)
      : handle_{std::move(other.handle_)},
        ref_table_{std::move(other.ref_table_)} {
    // adopt
  }

  constexpr std::optional<Rc<HandleType, RefTableType>> try_upgrade() const {
    if (ref_table_.try_weak_upgrade()) {
      return Rc<HandleType, RefTableType>::adopt(HandleType{handle_},
                                                 RefTableType{ref_table_});
    } else {
      return std::nullopt;
    }
  }

  ~WeakRc() {
    // weak release
    ref_table_.weak_deref();
  }

 private:
  constexpr WeakRc(HandleType&& handle, RefTableType&& ref_table)
      : handle_{std::move(handle)}, ref_table_{std::move(ref_table)} {}

  HandleType handle_;
  RefTableType ref_table_;
};

}  // namespace rc

template <typename T>
void launder(T*);

void b() {
  auto y = rc::Rc<int*>::adopt(nullptr, rc::VirtualRefTable{});
  launder(&y);
}



  static constexpr Unique own(HandleType&& handle, ManagerType&& manager) {
    return Unique{std::move(handle), std::move(manager)};
  }

  static constexpr Unique claim(HandleType&& handle, ManagerType&& manager) {
    manager.ref(handle);
    return own(std::move(handle), std::move(manager));
  }

    // take ownership of an already ref'd handle
  static constexpr Rc adopt(HandleType&& handle, ManagerType&& manager) {
    return Rc{std::move(handle), std::move(manager)};
  }

  // take ownership of a yet-to-be ref'd handle
  static constexpr Rc share(HandleType&& handle, ManagerType&& manager) {
    manager.ref(handle);
    return adopt(std::move(handle), std::move(manager));
  }