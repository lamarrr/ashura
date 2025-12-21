struct NonZeroConstraint
{
  static constexpr void check(auto);
};

struct NonNullConstraint
{
  static constexpr void check(auto);
};

struct Pow2Constraint
{
  static constexpr void check(auto);
};

struct AssumeConstrained
{
};

inline constexpr AssumeConstrained assume_constrained{};

// [ ] add tests
template <typename T, typename Constraint = NonZeroConstraint>
struct Constrained
{
  T value_;

  constexpr Constrained(T value) : value_{value}
  {
    Constraint::check(value);
  }

  constexpr Constrained(AssumeConstrained, T value) : value_{value}
  {
  }

  constexpr Constrained(Constrained const &)             = default;
  constexpr Constrained(Constrained &&)                  = default;
  constexpr Constrained & operator=(Constrained const &) = default;
  constexpr Constrained & operator=(Constrained &&)      = default;
  constexpr ~Constrained()                               = default;

  constexpr operator T const &() const
  {
    return value_;
  }

  constexpr void set(T value)
  {
    Constraint::check(value);
    value_ = value;
  }

  constexpr void set(AssumeConstrained, T value)
  {
    value_ = value;
  }

  constexpr T const & value() const
  {
    return value_;
  }
};

template <typename T>
using NonZero = Constrained<T, NonZeroConstraint>;

template <typename T>
using NonNull = Constrained<T, NonNullConstraint>;

template <typename T>
using Pow2 = Constrained<T, Pow2Constraint>;
