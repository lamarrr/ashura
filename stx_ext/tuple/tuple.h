

template <typename H, typename... T>
struct TupleStorage {
  H head;
  TupleStorage<T...> next;
};

template <typename H, typename... T>
struct TupleStorage {
  H head;
};

template <typename H, typename... T>
struct Tuple : TupleStorage<H, T...> {
Tuple( H&&, T&&... ) 

  H& get() &;
  H&& get() &&;
  H const& get() &&;
};
