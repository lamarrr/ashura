
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

/// a static storage manager handle represents a no-op. i.e. no operation is
/// required for managing lifetimes of the associated static resource.
///
/// thread-safe
///
struct StaticStorageManagerHandle final : public ManagerHandle {
  virtual void ref() override final {}
  virtual void unref() override final {}
};

static StaticStorageManagerHandle static_storage_manager_handle;

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
/// coincidentally, this should be able to support  reference counting
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

  /// on-move, the manager must copy and then invalidate the other
  /// manager's handle, the moved-from manager is required to be valid but
  /// unable to affect the associated state of the resource, i.e. (no-op). why
  /// not nullptr? nullptr means we'd have to perform a check everytime we want
  /// to send calls to the manager. but with a no-op we don't have any
  /// branches, though we'd have an extra copy on move (noop manager
  /// handle assignment).
  ///
  Manager(Manager&& other) : handle_{other.handle_} {
    /// unarm other and prevent it from affecting the state of any object
    other.handle_ = &noop_manager_handle;
  }

  Manager& operator=(Manager&& other) {
    std::swap(handle_, other.handle_);
    return *this;
  }

  void ref() const { handle_->ref(); }

  void unref() const { handle_->unref(); }

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
///
/// undefined behaviour to copy/move from/to a moved-from Rc
template <typename HandleType>
struct Rc {
  static_assert(is_resource_handle_type<HandleType>);

  using handle_type = HandleType;

  template <typename H>
  friend H const& unsafe_ref_handle(Rc<H> const&);

  template <typename H>
  friend H& unsafe_ref_handle(Rc<H>&);

  template <typename H>
  friend pmr::Manager const& unsafe_ref_manager(Rc<H> const&);

  template <typename H>
  friend pmr::Manager& unsafe_ref_manager(Rc<H>&);

  template <typename H>
  friend Rc<H> unsafe_make_rc(H&&, pmr::Manager&&);

  Rc(Rc const& other) : handle_{other.handle_}, manager_{other.manager_} {
    manager_.ref();
  }

  Rc(Rc&& other)
      : handle_{std::move(other.handle_)},
        manager_{std::move(other.manager_)} {}

  Rc& operator=(Rc const& other) {
    manager_.unref();
    other.manager_.ref();
    handle_ = other.handle_;
    manager_ = other.manager_;
    return *this;
  }

  Rc& operator=(Rc&& other) {
    std::swap(handle_, other.handle_);
    std::swap(manager_, other.manager_);
    return *this;
  }

  HandleType const& get() const { return handle_; }

  ~Rc() { manager_.unref(); }

 private:
  Rc(HandleType&& handle, pmr::Manager&& manager)
      : handle_{std::move(handle)}, manager_{std::move(manager)} {}

  HandleType handle_;
  pmr::Manager manager_;
};

template <typename H>
H const& unsafe_ref_handle(Rc<H> const& rc) {
  return rc.handle_;
}

template <typename H>
H& unsafe_ref_handle(Rc<H>& rc) {
  return rc.handle_;
}

template <typename H>
pmr::Manager const& unsafe_ref_manager(Rc<H> const& rc) {
  return rc.manager_;
}

template <typename H>
pmr::Manager& unsafe_ref_manager(Rc<H>& rc) {
  return rc.manager_;
}

template <typename H>
Rc<H> unsafe_make_rc(H&& handle, pmr::Manager&& manager) {
  return Rc<H>{std::move(handle), std::move(manager)};
}

/// Transmute a resource that uses a polymorphic manager.
/// transmutation involves pretending that a target resource constructed from
/// another source resource is valid provided the other source resource is
/// valid.
///
/// This is more of an alias or possibly unsafe alias as we can't guarantee its
/// validity.
///
/// i.e. `Rc<string_view, pmr::Manager>` can transmute `Rc<std::string *,
/// pmr::Manager>`. this means the contained `string_view` is valid as long as
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
Rc<Target> transmute(Target target, Rc<Source>&& source) {
  return unsafe_make_rc(std::move(target),
                        std::move(unsafe_ref_manager(source)));
}

template <typename Target, typename Source>
Rc<Target> transmute(Target target, Rc<Source> const& source) {
  return transmute<Target, Source>(std::move(target), Rc<Source>(source));
}
}  // namespace stx
