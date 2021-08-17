
#pragma once
#include <type_traits>
#include <utility>

#include "stx/manager.h"

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

/// Rc - reference-counted resource
///
/// primarily intended for dynamic lifetime management dispatch
///
/// NOTE: our `RcPtr` does not accept nullptr and can't be
/// a nullptr. if you need a nullable RcPtr, consider
/// `Option<RcPtr>` or use `std::shared_ptr`
///
///
/// undefined behaviour to copy/move from/to a moved-from Rc
///
/// NOTE: Rc is neither a pointer nor a function. It just does one thing:
/// manages lifetime
///
template <typename HandleType>
struct Rc {
  static_assert(is_resource_handle_type<HandleType>);

  using handle_type = HandleType;

  constexpr Rc(HandleType&& handle, Manager&& manager)
      : handle_{std::move(handle)}, manager_{std::move(manager)} {}

  Rc(Rc const& other) : handle_{other.handle_}, manager_{other.manager_} {
    manager_.ref();
  }

  constexpr Rc(Rc&& other) = default;
  constexpr Rc& operator=(Rc&& other) = default;

  // TODO(lamarrr): test self-assignment
  Rc& operator=(Rc const& other) {
    // ref needs to happen before unref, in case the rc refers to itself
    other.manager_.ref();
    manager_.unref();
    handle_ = other.handle_;
    manager_ = other.manager_;
    return *this;
  }

  ~Rc() { manager_.unref(); }

  constexpr HandleType const& get() const { return handle_; }

  constexpr Rc share() const { return *this; }

  constexpr auto& unsafe_handle_ref() { return handle_; }

  constexpr auto const& unsafe_handle_ref() const { return handle_; }

  constexpr auto& unsafe_manager_ref() { return manager_; }

  constexpr auto const& unsafe_manager_ref() const { return manager_; }

 private:
  HandleType handle_;
  Manager manager_;
};

template <typename H>
constexpr Rc<H> unsafe_make_rc(H&& handle, Manager&& manager) {
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
constexpr Rc<Target> transmute(Target target, Rc<Source>&& source) {
  return unsafe_make_rc(std::move(target),
                        std::move(source.unsafe_manager_ref()));
}

template <typename Target, typename Source>
Rc<Target> transmute(Target target, Rc<Source> const& source) {
  return transmute<Target, Source>(std::move(target), Rc<Source>{source});
}
}  // namespace stx
