#include <memory>

namespace vlk {
namespace ui {

template <typename T>
struct FixedVector {

 private:
  std::unique_ptr<T[]> content_;
  size_t size_;
};

}  // namespace ui
};  // namespace vlk
