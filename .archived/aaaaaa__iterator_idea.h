
template <typename T>
T* begin(Flex<T>& flex) {
  return flex.iterator____begin();
}

template <typename T>
T* begin(Flex<T> const& flex) {
  return flex.iterator____begin();
}

template <typename T>
T* begin(Flex<T>&& flex) = delete;

template <typename T>
T* cbegin(Flex<T>& flex) {
  return flex.iterator____begin();
}

template <typename T>
T* cbegin(Flex<T> const& flex) {
  return flex.iterator____begin();
}

template <typename T>
T* cbegin(Flex<T>&& flex) = delete;

template <typename T>
T* end(Flex<T>& flex) {
  return flex.iterator____begin();
}

template <typename T>
T* end(Flex<T> const& flex) {
  return flex.iterator____begin();
}

template <typename T>
T* end(Flex<T>&& flex) = delete;

template <typename T>
T* cend(Flex<T>& flex) {
  return flex.iterator____cend();
}

template <typename T>
T* cend(Flex<T> const& flex) {
  return flex.iterator____cend();
}

template <typename T>
T* cend(Flex<T>&& flex) = delete;
