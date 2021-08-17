#include <cinttypes>

namespace vlk {

struct Asset {
  explicit constexpr Asset(uint64_t size_in_bytes)
      : size_bytes_{size_in_bytes} {}

  virtual ~Asset() = 0;

  constexpr uint64_t size_bytes() const { return size_bytes_; }

 private:
  uint64_t size_bytes_ = 0;
};

};  // namespace vlk
