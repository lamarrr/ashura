


namespace vlk{


template <typename Target, typename Source>
STX_FORCE_INLINE Target upcast(Source &source) {
  static_assert(std::is_lvalue_reference_v<Target>);
  auto *const dyn_ptr =
      dynamic_cast<std::remove_reference_t<Target> *>(&source);
  VLK_ENSURE(dyn_ptr != nullptr, "Dynamic upcast failed");
  return *dyn_ptr;
}


template <typename Target, typename Source>
STX_FORCE_INLINE Target upcast_fallible(Source &source) {
  static_assert(std::is_lvalue_reference_v<Target>);
  auto *const dyn_ptr =
      dynamic_cast<std::remove_reference_t<Target> *>(&source);
  VLK_ENSURE(dyn_ptr != nullptr, "Dynamic upcast failed");
  return *dyn_ptr;
}

}