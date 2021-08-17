
#pragma once
#include <type_traits>
#include <utility>

namespace stx {
/// Handles refer to a representation of a resource that needs to be managed.
/// this resource can be memory, C-API resource, etc.
///
///
/// Handle types are just values to be passed and moved around and whose
/// validity is guaranteed by the manager, i.e. the manager determines if a
/// nullptr is a valid memory resource handle and handles it appropriately.
///
template <typename T>
constexpr bool is_resource_handle_type =
    std::is_copy_constructible_v<T>&& std::is_move_constructible_v<T>&&
        std::is_copy_assignable_v<T>&& std::is_move_assignable_v<T>;

/// Nice-to-have: manager traits for manager.ref(handle) and
/// manager.unref(handle)
template <typename T>
constexpr bool is_manager_type =
    std::is_copy_constructible_v<T>&& std::is_move_constructible_v<T>&&
        std::is_copy_assignable_v<T>&& std::is_move_assignable_v<T>;

namespace pmr {
/// A handle/abstract interface to a polymorphic resource manager.
///
/// the operations are specified atomically so they can be used for reftable
/// implementations that choose to use atomic (multi-threaded) or non-atomic
/// operations (single-threaded) for synchronization. Implementations can also
/// use pool-based management.
///
///
/// thread-safety depends on implementation.
///
struct ManagerHandle {
  /// increase the strong ref count of the associated resource.
  /// ensure resource is valid before this is called.
  /// a resource with a refcount of 1 or more must always be valid.
  ///
  virtual void ref() = 0;
  /// reduce the ref count of the associated resource.
  ///
  /// a resource with a refcount of 0 needs not be valid.
  ///
  /// the manager handle is not required to be valid once the resource ref count
  /// becomes 0.
  ///
  virtual void unref() = 0;
};

/// a static (usually static storage) manager handle. represents a no-op. i.e.
/// no operation is required for managing lifetimes of the associated static
/// resource.
///
/// thread-safe
///
struct StaticManagerHandle final : public ManagerHandle {
  virtual void ref() override final {}
  virtual void unref() override final {}
};

static StaticManagerHandle static_manager_handle;

/// this handle type has no effect on the state of the program.
/// we used this to help prevent cases where we have to perform branches to
/// check validity of the manager handle.
///
/// thread-safe
///
struct NoopManagerHandle final : public ManagerHandle {
  virtual void ref() override final {}
  virtual void unref() override final {}
};

static NoopManagerHandle noop_manager_handle;

///
/// this is a polymorphic resource manager.
/// the resource can be a part of the manager
/// (intrusive/self-managed) or even be externally located (non-intrusive, or
/// separate control block). management of the resource can be intrusive or
/// non-intrusive which makes it flexible.
///
/// the manager is free to delete itself once the resource ref count reaches
/// 0. the manager can also delegate the destruction of itself
/// and its associated resource. i.e. delegating resource management to a memory
/// pool segment or bulk-allocated memory segment.
///
/// the resource management is decoupled from the resource or control block.
///
/// this enables a couple of use-cases:
///
/// - use in embedded systems (via static storage and static memory pools)
/// - use in single-threaded environments where ref-counting might not be
/// needed.
/// - use in scenarios where the user is certain the resource will always
/// outlive the Rc
/// - usage with custom memory management solutions (i.e. pool/bulk-based
/// solutions)
///
/// this is a scalable abstraction over resource management, ***I think***.
///
/// coincidentally, this should be able to support constexpr reference counting
/// by swapping out the `ManagerType` arguments of the Rc, how that'd be useful,
/// I'm still figuring out.
///
///
/// resource handles can be of any type. not just pointers which is only what
/// `shared_ptr` supports.
///
///
struct Manager {
  explicit Manager(ManagerHandle& handle) : handle_{&handle} {}

  /// default-initialized with a no-op handle.
  /// this will not cause a fatal crash (as would happen if we used a  nullptr).
  Manager() : handle_{&noop_manager_handle} {}

  /// on-copy, the handles must refer to the same manager
  Manager(Manager const& other) = default;
  Manager& operator=(Manager const& other) = default;

  /// on-move, the reference table must copy and then invalidate the other
  /// refrence table's handle, the moved-from table is required to be valid but
  /// unable to affect the associated state of the resource, i.e. (no-op). why
  /// not nullptr? nullptr means we'd have to perform a check everytime we want
  /// to send calls to the reftable (weak_ref, strong_ref, weak_deref,
  /// strong_deref, try_weak_upgrade). but with a no-op we don't have any
  /// branches, though we'd have an extra copy on move (new noop reftable
  /// pointer assignment).
  ///
  Manager(Manager&& other) : handle_{other.handle_} {
    /// unarm other and prevent it from affecting the state of any object
    other.handle_ = &noop_manager_handle;
  }

  Manager& operator=(Manager&& other) {
    std::swap(handle_, other.handle_);
    return *this;
  }

  template <typename HandleType>
  void ref(HandleType const&) const {
    handle_->ref();
  }

  template <typename HandleType>
  void unref(HandleType const&) const {
    handle_->unref();
  }

 private:
  ManagerHandle* handle_;
};

}  // namespace pmr

// a resource manager should be able to handle its handle's default
// state...!!!!!!!??????????

/// Rc - reference-counted resource
///
/// primarily intended for dynamic dispatch
///
/// NOTE: our `RcPtr` does not accept nullptr and can't be
/// a nullptr. if you need a nullable RcPtr, consider
/// `Option<RcPtr>` or use `std::shared_ptr`
///
template <typename HandleType, typename ManagerType>
struct Rc {
  static_assert(is_resource_handle_type<HandleType>);
  static_assert(is_manager_type<ManagerType>);

  using handle_type = HandleType;
  using manager_type = ManagerType;
  using handle_traits_type = HandleTraitsType;

  template <typename H, typename M>
  friend constexpr H const& get_handle(Rc<H, M> const& rc);

  template <typename H, typename M>
  friend constexpr H&& unsafe_move_handle(Rc<H, M>&& rc);

  template <typename H, typename M>
  friend constexpr M const& get_manager(Rc<H, M> const& rc);

  template <typename H, typename M>
  friend constexpr M&& unsafe_move_manager(Rc<H, M>&& rc);

  // UNSAFE, you really probably shouldn't be using this. consider the helper
  // functions instead.
  constexpr Rc(HandleType&& handle, ManagerType&& manager)
      : handle_{HandleTraitsType::move(std::move(handle))},
        manager_{std::move(manager)} {}

  constexpr Rc(Rc const& other)
      : handle_{HandleTraitsType::copy(other.handle_)},
        manager_{other.manager_} {
    other.manager_.ref(other.handle_);
  }

  constexpr Rc(Rc&& other)
      : handle_{HandleTraitsType::move(std::move(other.handle))},
        manager_{std::move(other.manager_)} {}

  constexpr Rc& operator=(Rc const& other) {
    manager_.unref(handle_);
    other.manager_.ref(other.handle_);
    handle_ = other.handle_;
    manager_ = other.manager_;
    return *this;
  }

  constexpr Rc& operator=(Rc&& other) {
    std::swap(handle_, other.handle_);
    std::swap(manager_, other.manager_);
    return *this;
  }

  constexpr HandleType const& get() const { return handle_; }

  /// release
  ~Rc() { manager_.unref(handle_); }

 private:
  HandleType handle_;
  ManagerType manager_;
};

/// intended for static dispatch
///
/// Unique - unique-ly owned resource
///
/// NOTE: our `UniquePtr` does not accept nullptr and can't be
/// a nullptr. if you need a nullable `UniquePtr`, consider
/// `Option<UniquePtr>` or use `std::unique_ptr`
///
template <typename HandleType, typename ManagerType>
struct Unique {
  static_assert(is_resource_handle_type<HandleType>);
  static_assert(is_manager_type<ManagerType>);

  using handle_type = HandleType;
  using manager_type = ManagerType;

  template <typename H, typename M>
  friend constexpr H const& get_handle(Rc<H, M> const& rc);

  template <typename H, typename M>
  friend constexpr H&& unsafe_move_handle(Rc<H, M>&& rc);

  template <typename H, typename M>
  friend constexpr M const& get_manager(Rc<H, M> const& rc);

  template <typename H, typename M>
  friend constexpr M&& unsafe_move_manager(Rc<H, M>&& rc);

  // UNSAFE, you really probably shouldn't be using this. consider the helper
  // functions instead.
  constexpr Unique(HandleType&& handle, ManagerType&& manager)
      : handle_{std::move(handle)}, manager_{std::move(manager)} {}

  // we still need null ptr???? else double free
  constexpr Unique(Unique&& other)
      : handle_{move_handle(other.handle_)},
        manager_{std::move(other.manager_)} {}

  constexpr Unique& operator=(Unique&& other) {
    std::swap(manager_, other.manager_);
    std::swap(handle_, other.handle_);
    return *this;
  }

  Unique(Unique const&) = delete;
  Unique& operator=(Unique const&) = delete;

  constexpr HandleType const& get() const { return handle_; }

  ~Unique() { manager_.unref(handle_); }

 private:
  HandleType handle_;
  ManagerType manager_;
};

template <typename H, typename M>
constexpr H const& get_handle(Rc<H, M> const& rc) {
  return rc.handle_;
}

template <typename H, typename M>
constexpr H&& unsafe_move_handle(Rc<H, M>&& rc) {
  return std::move(rc.handle_);
}

template <typename H, typename M>
constexpr M const& get_manager(Rc<H, M> const& rc) {
  return rc.manager_;
}

template <typename H, typename M>
constexpr M&& unsafe_move_manager(Rc<H, M>&& rc) {
  return std::move(rc.manager_);
}

template <typename H, typename M>
constexpr H const& get_handle(Unique<H, M> const& unique) {
  return unique.handle_;
}

template <typename H, typename M>
constexpr H&& unsafe_move_handle(Unique<H, M>&& unique) {
  return std::move(unique.handle_);
}

template <typename H, typename M>
constexpr M const& get_manager(Unique<H, M> const& unique) {
  return unique.manager_;
}

template <typename H, typename M>
constexpr M&& unsafe_move_manager(Unique<H, M>&& unique) {
  return std::move(unique.manager_);
}

/// Transmute a resource that uses a polymorphic manager.
/// transmutation involves pretending that a target resource constructed from
/// another source resource is valid provided the other source resource is
/// valid.
///
/// This is more of an alias or possibly unsafe alias as we can't guarantee its
/// validity.
///
/// i.e. `Rc<string_view, Manager>` can transmute `Rc<std::string *,
/// Manager>`. this means the contained `string_view` is valid as long as
/// the string pointer is valid.
///
///
/// NOTE: transmuting a Rc handle means the manager knows how to handle the
/// resource without using the resource handle. which is the case for resources
/// that use a polymorphic manager but not so for resources with non-polymorphic
/// managers.
/// this functions similar similar to `std::shared_ptr's` aliasing constructors.
///
///
template <typename Target, typename Source>
Rc<Target, Manager> transmute(Target&& target,
                                   Rc<Source, Manager>&& source) {
  return Rc<Target, Manager>{std::move(target),
                                  unsafe_move_manager(std::move(source))};
}

template <typename Target, typename Source>
Rc<Target, Manager> transmute(Target&& target,
                                   Rc<Source, Manager> const& source) {
  get_manager(source).ref(target);
  return Rc<Target, Manager>{std::move(target),
                                  Manager{get_manager(source)}};
}

namespace pmr {
template <typename T>
using Rc = stx::Rc<T, stx::Manager>;
}

// TODO(lamarrr): transmute unique

}  // namespace stx
