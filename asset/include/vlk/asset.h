#include <cinttypes>

namespace vlk {

// this is bad and doesn't represent failure or success
struct Asset {
  Asset() : size_bytes_{0} {}

  virtual ~Asset() = 0;

  uint64_t size_bytes() const { return size_bytes_; }

 protected:
  void update_size(uint64_t size) { size_bytes_ = size; }

 private:
  uint64_t size_bytes_ = 0;
};

};  // namespace vlk
