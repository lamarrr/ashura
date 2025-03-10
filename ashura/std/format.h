/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/buffer.h"
#include "ashura/std/math.h"
#include "ashura/std/types.h"

namespace ash
{
namespace fmt
{

static constexpr usize MAX_WIDTH     = 254;
static constexpr usize MAX_PRECISION = 254;

static constexpr usize NONE_WIDTH     = 255;
static constexpr usize NONE_PRECISION = 255;

enum class Style : char
{
  Default    = 'd',
  Decimal    = 'd',
  Octal      = 'o',
  Hex        = 'x',
  Binary     = 'b',
  Scientific = 'f'
};

/// Syntax: [sign][alternate_form][width].[precision][style]
///
/// sign: '+'
/// alternate_form: '#'
/// width: 0-MAX_WIDTH
/// precision-separator: '.'
/// precision: 0-MAX_PRECISION
/// style: `d`, `o`, `x`, `b`, `s`
///
/// i.e: {+#4.5x}
struct Spec
{
  Style style              = Style::Default;
  bool  sign           : 1 = true;
  bool  alternate_form : 1 = false;
  u8    width              = NONE_WIDTH;
  u8    precision          = NONE_PRECISION;
};

typedef Fn<void(Span<char const>)> Sink;

}    // namespace fmt

template <typename T>
constexpr auto format(fmt::Sink sink, fmt::Spec spec, T const & obj)
  -> decltype(obj.format(sink, spec))
{
  return obj.format(sink, spec);
}

template <typename T>
constexpr void format(fmt::Sink sink, fmt::Spec, T const &)
{
  sink("[?]"_str);
}

namespace fmt
{

static constexpr usize MAX_ARGS = 64;

enum class [[nodiscard]] Error : u8
{
  None            = 0,
  OutOfMemory     = 1,
  UnexpectedToken = 2,
  ItemsMismatch   = 3,
  UnmatchedToken  = 4
};

constexpr Span<char const> to_str(Error e)
{
  switch (e)
  {
    case Error::None:
      return "None"_str;
    case Error::OutOfMemory:
      return "OutOfMemory"_str;
    case Error::UnexpectedToken:
      return "UnexpectedToken"_str;
    case Error::ItemsMismatch:
      return "ItemsMismatch"_str;
    case Error::UnmatchedToken:
      return "UnmatchedToken"_str;
    default:
      return "Unrecognized"_str;
  }
}

typedef void (*Formatter)(Sink, Spec spec, void const * obj);

template <typename T>
inline constexpr Formatter formatter_of =
  [](Sink sink, Spec spec, void const * p_obj) {
    T const & obj = *reinterpret_cast<T const *>(p_obj);
    format(sink, spec, obj);
  };

struct FormatArg
{
  Formatter    formatter = noop;
  void const * obj       = nullptr;

  template <typename T>
  static constexpr FormatArg from(T const & obj)
  {
    return FormatArg{.formatter = formatter_of<T>, .obj = &obj};
  }

  constexpr void format(Sink sink, Spec spec) const
  {
    formatter(sink, spec, obj);
  }
};

struct [[nodiscard]] Result
{
  Error error    = Error::None;
  Slice position = {};
};

enum class ParseState : u32
{
  Start              = 0,
  Finished           = 1,
  Sign               = 2,
  AlternateForm      = 3,
  Width              = 4,
  PrecisionSeparator = 5,
  Precision          = 6,
  Style              = 7,
  Error              = 8
};

enum class TokenType : u32
{
  None          = 0,
  Sign          = 1,
  AlternateForm = 2,
  Number        = 3,
  Dot           = 4,
  Style         = 5,
  Unrecognized  = 6,
  Finished      = 7
};

enum class OpType : u8
{
  Str = 0,
  Fmt = 1
};

struct Op
{
  OpType type = OpType::Fmt;
  Spec   spec = {};
  Slice  pos  = {};
};

constexpr bool is_numeric(char c)
{
  return c >= '0' && c <= '9';
}

constexpr bool is_sign(char c)
{
  return c == '+';
}

constexpr bool is_alternate(char c)
{
  return c == '#';
}

constexpr bool is_dot(char c)
{
  return c == '.';
}

constexpr bool is_white_space(char c)
{
  return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

constexpr bool is_alpha(char c)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

struct [[nodiscard]] TokenSeekResult
{
  TokenType type  = TokenType::Unrecognized;
  Slice     token = {};
};

constexpr TokenType next_token(char const *& iter, char const * const end)
{
  if (is_sign(*iter))
  {
    iter++;
    return TokenType::Sign;
  }
  else if (is_alternate(*iter))
  {
    iter++;
    return TokenType::AlternateForm;
  }
  else if (is_numeric(*iter))
  {
    iter++;
    while (iter != end && is_numeric(*iter))
    {
      iter++;
    }
    return TokenType::Number;
  }
  else if (is_dot(*iter))
  {
    iter++;
    return TokenType::Dot;
  }
  else if (is_alpha(*iter))
  {
    iter++;
    return TokenType::Style;
  }
  else
  {
    iter++;
    return TokenType::Unrecognized;
  }
}

constexpr TokenSeekResult seek_token(Span<char const> source,
                                     char const *& iter, char const * const end)
{
  if (is_white_space(*iter))
  {
    while (iter != end && is_white_space(*iter))
    {
      iter++;
    }
  }

  if (iter != end)
  {
    auto const      token_begin = iter;
    TokenType const type        = next_token(iter, end);
    return TokenSeekResult{.type = type,
                           .token{Span{token_begin, iter}.as_slice_of(source)}};
  }
  else
  {
    return TokenSeekResult{
      .type = TokenType::Finished, .token{0, 0}
    };
  }
}

constexpr ParseState parser_state(ParseState state, TokenType token)
{
  switch (state)
  {
    case ParseState::Start:
      switch (token)
      {
        case TokenType::Finished:
          return ParseState::Finished;
        case TokenType::Sign:
          return ParseState::Sign;
        case TokenType::AlternateForm:
          return ParseState::AlternateForm;
        case TokenType::Number:
          return ParseState::Width;
        case TokenType::Dot:
          return ParseState::PrecisionSeparator;
        case TokenType::Style:
          return ParseState::Style;
        default:
          return ParseState::Error;
      }
      break;
    case ParseState::Sign:
      switch (token)
      {
        case TokenType::Finished:
          return ParseState::Finished;
        case TokenType::AlternateForm:
          return ParseState::AlternateForm;
        case TokenType::Number:
          return ParseState::Width;
        case TokenType::Dot:
          return ParseState::PrecisionSeparator;
        case TokenType::Style:
          return ParseState::Style;
        default:
          return ParseState::Error;
      }
      break;
    case ParseState::AlternateForm:
      switch (token)
      {
        case TokenType::Finished:
          return ParseState::Finished;
        case TokenType::Number:
          return ParseState::Width;
        case TokenType::Dot:
          return ParseState::PrecisionSeparator;
        case TokenType::Style:
          return ParseState::Style;
        default:
          return ParseState::Error;
      }
      break;
    case ParseState::Width:
      switch (token)
      {
        case TokenType::Finished:
          return ParseState::Finished;
        case TokenType::Number:
          return ParseState::Width;
        case TokenType::Dot:
          return ParseState::PrecisionSeparator;
        case TokenType::Style:
          return ParseState::Style;
        default:
          return ParseState::Error;
      }
      break;
    case ParseState::PrecisionSeparator:
      switch (token)
      {
        case TokenType::Finished:
          return ParseState::Finished;
        case TokenType::Number:
          return ParseState::Precision;
        case TokenType::Style:
          return ParseState::Style;
        default:
          return ParseState::Error;
      }
      break;
    case ParseState::Precision:
      switch (token)
      {
        case TokenType::Finished:
          return ParseState::Finished;
        case TokenType::Style:
          return ParseState::Style;
        default:
          return ParseState::Error;
      }
      break;
    case ParseState::Style:
      switch (token)
      {
        case TokenType::Finished:
          return ParseState::Finished;
        default:
          return ParseState::Error;
      }
      break;
    default:
      return ParseState::Error;
  }
}

[[nodiscard]] constexpr bool unsafe_parse_u64(Span<char const> str, u64 & i)
{
  i = 0;

  // 18446744073709551615
  if (str.size() > 20)
  {
    return false;
  }

  char const * iter = str.pbegin();
  auto const   end  = str.pend();

  while (iter != end)
  {
    i = i * 10U + (*iter - '0');
    iter++;
  }

  return true;
}

constexpr void consume_token(Span<char const> str, ParseState state,
                             Spec & spec)
{
  switch (state)
  {
    case ParseState::Sign:
    {
      spec.sign = true;
    }
    break;
    case ParseState::AlternateForm:
    {
      spec.alternate_form = true;
    }
    break;
    case ParseState::Width:
    {
      u64 width = 0;
      if (!unsafe_parse_u64(str, width))
      {
        width = MAX_WIDTH + 1;
      }
      else
      {
        width = min(width, MAX_WIDTH);
      }

      spec.width = width;
    }
    break;
    case ParseState::Precision:
    {
      u64 precision = 0;
      if (!unsafe_parse_u64(str, precision))
      {
        precision = MAX_PRECISION + 1;
      }
      else
      {
        precision = min(precision, MAX_PRECISION);
      }

      spec.precision = precision;
    }
    break;
    case ParseState::Style:
    {
      spec.style = Style{str[0]};
    }
    break;
    default:
      break;
  }
}

constexpr Result parse_spec(Span<char const> source, Span<char const> str,
                            Spec & spec)
{
  spec = {};

  ParseState state = ParseState::Start;

  char const * iter = str.pbegin();
  auto const   end  = str.pend();

  while (iter != end)
  {
    TokenSeekResult const result = seek_token(source, iter, end);

    if (result.type == TokenType::Unrecognized)
    {
      return Result{.error = Error::UnexpectedToken, .position = result.token};
    }

    ParseState const current_state = parser_state(state, result.type);

    switch (current_state)
    {
      case ParseState::Finished:
        break;
      case ParseState::Error:
      {
        return Result{.error    = Error::UnexpectedToken,
                      .position = result.token};
      }
      default:
      {
        consume_token(source.slice(result.token), current_state, spec);
        state = current_state;
      }
      break;
    }
  }

  return Result{.error = Error::None};
}

template <typename T>
constexpr bool streq_same_size(Span<T> s0, Span<T> s1)
{
  auto       p0     = s0.pbegin();
  auto const p0_end = s0.pend();
  auto       p1     = s1.pbegin();

  while (p0 != p0_end && *p0 == *p1)
  {
    p0++;
    p1++;
  }

  return p0 == p0_end;
}

template <typename T>
constexpr Slice substr(Span<T> str, Span<T> part)
{
  auto const part_size = part.size();

  if (part_size > str.size())
  {
    return Slice{USIZE_MAX, 0};
  }

  if (part_size == 0)
  {
    return Slice{0, 0};
  }

  auto const begin    = str.pbegin();
  auto       iter     = str.pbegin();
  auto const iter_end = str.pend() - part.size();

  while (iter != iter_end && !streq_same_size({iter, part_size}, part))
  {
    iter++;
  }

  if (iter == iter_end)
  {
    return Slice{USIZE_MAX, 0};
  }

  return Slice{static_cast<usize>(iter - begin), part_size};
}

constexpr Result push_spec(Span<char const> format, Span<char const> spec_src,
                           Buffer<Op> & ops, usize & num_args)
{
  Spec spec;
  if (auto result = parse_spec(format, spec_src, spec);
      result.error != Error::None)
  {
    return result;
  }

  if (!ops.push(Op{.type = OpType::Fmt,
                   .spec = spec,
                   .pos  = spec_src.as_slice_of(format)}))
  {
    return Result{.error = Error::OutOfMemory};
  }

  num_args++;

  return Result{.error = Error::None};
}

constexpr char const * seek(char const * iter, char const * const end, char c)
{
  while (iter != end && *iter != c)
  {
    iter++;
  }
  return iter;
}

constexpr char const * seek_ne(char const * iter, char const * const end,
                               char c)
{
  while (iter != end && *iter == c)
  {
    iter++;
  }
  return iter;
}

constexpr char const * seek_n(char const * iter, char const * const end, char c,
                              usize n)
{
  while (iter != end)
  {
    while (iter != end && *iter != c)
    {
      iter++;
    }

    auto const match_begin = iter;

    while (iter != end && *iter == c)
    {
      iter++;
    }

    if (static_cast<usize>(iter - match_begin) == n)
    {
      return match_begin;
    }
  }

  return end;
}

constexpr Result parse(Span<char const> format, Buffer<Op> & ops,
                       usize & num_args)
{
  char const * iter = format.pbegin();
  auto const   end  = format.pend();

  while (iter != end)
  {
    auto const seek_begin = iter;
    iter                  = seek(iter, end, '{');

    if (seek_begin != iter)
    {
      if (!ops.push(Op{.type = OpType::Str,
                       .pos{Span{seek_begin, iter}.as_slice_of(format)}}))
      {
        return Result{.error = Error::OutOfMemory};
      }
    }

    if (iter == end)
    {
      continue;
    }

    auto const open_brace_begin = iter++;

    iter = seek_ne(iter, end, '{');

    auto const open_brace_end = iter;

    auto const brace_level =
      static_cast<usize>(open_brace_end - open_brace_begin);

    switch (brace_level)
    {
      case 1:
      {
        iter = seek(iter, end, '}');

        if (iter == end)
        {
          return Result{
            .error = Error::UnmatchedToken,
            .position =
              Span{open_brace_begin, brace_level}
              .as_slice_of(format)
          };
        }

        Span const spec{open_brace_end, iter};

        iter++;

        if (auto result = push_spec(format, spec, ops, num_args);
            result.error != Error::None)
        {
          return result;
        }
      }
      break;

      default:
      {
        iter = seek_n(iter, end, '}', brace_level);

        if (iter == end)
        {
          return Result{
            .error = Error::UnmatchedToken,
            .position =
              Span{open_brace_begin, brace_level}
              .as_slice_of(format)
          };
        }

        auto const close_brace_begin = iter;

        iter += brace_level;

        if (!ops.push(Op{
              .type = OpType::Str,
              .pos =
                Span{open_brace_end, close_brace_begin}
                .as_slice_of(format)
        }))
        {
          return Result{.error = Error::OutOfMemory};
        }
      }
      break;
    }
  }

  return Result{.error = Error::None};
}

struct Context
{
  Sink             sink_;
  Span<char const> fstr_;
  Buffer<Op>       ops_;
  usize            num_args_;

  constexpr Context(Sink sink, Buffer<Op> buffer) :
    sink_{sink},
    fstr_{},
    ops_{std::move(buffer)},
    num_args_{0}
  {
  }

  constexpr Context(Context const &)             = delete;
  constexpr Context(Context &&)                  = default;
  constexpr Context & operator=(Context const &) = delete;
  constexpr Context & operator=(Context &&)      = default;
  constexpr ~Context()                           = default;

  constexpr Result parse(Span<char const> fstr)
  {
    fstr_ = fstr;
    ops_.clear();
    num_args_ = 0;
    return fmt::parse(fstr_, ops_, num_args_);
  }

  template <typename... T>
  constexpr Result execute(T const &... args) const
  {
    Array<FormatArg, sizeof...(T)> const fmt_args{FormatArg::from(args)...};

    return execute_span(fmt_args);
  }

  constexpr Result execute_span(Span<FormatArg const> args) const
  {
    if (num_args_ != args.size())
    {
      return Result{.error = Error::ItemsMismatch};
    }

    usize arg = 0;

    for (Op const & op : ops_)
    {
      switch (op.type)
      {
        case OpType::Fmt:
        {
          args[arg].format(sink_, op.spec);
          arg++;
        }
        break;
        case OpType::Str:
        {
          sink_(fstr_.slice(op.pos));
        }
        break;
        default:
          break;
      }
    }

    return Result{.error = Error::None};
  }

  template <typename... T>
  constexpr Result format(Span<char const> fstr, T const &... args)
  {
    if (Result result = parse(fstr); result.error != Error::None)
    {
      return result;
    }

    return execute(args...);
  }
};

};    // namespace fmt

void format(fmt::Sink sink, fmt::Spec, bool const & value);
void format(fmt::Sink sink, fmt::Spec, u8 const & value);
void format(fmt::Sink sink, fmt::Spec, u16 const & value);
void format(fmt::Sink sink, fmt::Spec, u32 const & value);
void format(fmt::Sink sink, fmt::Spec, u64 const & value);
void format(fmt::Sink sink, fmt::Spec, i8 const & value);
void format(fmt::Sink sink, fmt::Spec, i16 const & value);
void format(fmt::Sink sink, fmt::Spec, i32 const & value);
void format(fmt::Sink sink, fmt::Spec, i64 const & value);
void format(fmt::Sink sink, fmt::Spec, f32 const & value);
void format(fmt::Sink sink, fmt::Spec, f64 const & value);
void format(fmt::Sink sink, fmt::Spec, Vec2 const & value);
void format(fmt::Sink sink, fmt::Spec, Vec3 const & value);
void format(fmt::Sink sink, fmt::Spec, Vec4 const & value);
void format(fmt::Sink sink, fmt::Spec, Vec2I const & value);
void format(fmt::Sink sink, fmt::Spec, Vec3I const & value);
void format(fmt::Sink sink, fmt::Spec, Vec4I const & value);
void format(fmt::Sink sink, fmt::Spec, Vec2U const & value);
void format(fmt::Sink sink, fmt::Spec, Vec3U const & value);
void format(fmt::Sink sink, fmt::Spec, Vec4U const & value);
void format(fmt::Sink sink, fmt::Spec, Span<char const> const & str);
void format(fmt::Sink sink, fmt::Spec, Span<char> const & str);

template <usize N>
void format(fmt::Sink sink, fmt::Spec spec, char (&str)[N])
{
  format(sink, spec, span(str));
}

template <usize N>
void format(fmt::Sink sink, fmt::Spec spec, char const (&str)[N])
{
  format(sink, spec, span(str));
}

void format(fmt::Sink sink, fmt::Spec, void const * const & str);

template <typename T>
void format(fmt::Sink sink, fmt::Spec spec, T * const & ptr)
{
  return format(sink, spec, (void const *) ptr);
}

}    // namespace ash
