/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/types.h"

namespace ash
{

namespace intr
{

namespace list
{

/// @brief Unlink Node `head` from the List, producing a new one-element List
/// @tparam T
/// @param head must be valid and non-null
/// @return popped list, never null
template <typename Node, Node * Node::* prev, Node * Node::* next>
constexpr void unlink(Node * head)
{
  // detach from siblings
  head->*next->*prev = head->*prev;
  head->*prev->*next = head->*next;
  // create 1 element node
  head->*next        = head;
  head->*prev        = head;
}

/// @brief Remove from the front of the list, producing a new one-element List
/// @tparam T
/// @param head must be valid and non-null, set to null if empty
/// @return unlinked element or null
template <typename Node, Node * Node::* prev, Node * Node::* next>
[[nodiscard]] constexpr Node * unlink_front(Node *& head)
{
  Node * out      = head;
  Node * new_head = (head->*next == head) ? nullptr : head->*next;
  unlink<Node, prev, next>(out);
  head = new_head;
  return out;
}

/// @brief Remove from the back of the list, producing a new one-element List
/// @tparam T
/// @param head must be valid and non-null, set to null if empty
/// @return unlinked element or null
template <typename Node, Node * Node::* prev, Node * Node::* next>
[[nodiscard]] constexpr Node * unlink_back(Node *& head)
{
  Node * out      = head->*prev;
  Node * new_head = (head->*prev == head) ? nullptr : head;
  unlink<Node, prev, next>(out);
  head = new_head;
  return out;
}

/// @brief Attach List `ext` to the end of List `head`
///
/// @tparam T
/// @param head must be valid and non-null
/// @param ext must be valid and non-null
///
template <typename Node, Node * Node::* prev, Node * Node::* next>
constexpr void link(Node * ASH_RESTRICT head, Node * ASH_RESTRICT ext)
{
  Node * node_head = head;
  Node * node_tail = head->*prev;
  Node * ext_head  = ext;
  Node * ext_tail  = ext->*prev;
  ext_head->*prev  = node_tail;
  ext_tail->*next  = node_head;
  node_head->*prev = ext_tail;
  node_tail->*next = ext_head;
}

/// @brief Attach List `ext` to the back of List `head`, using `head` as anchor
///
/// @param head must be valid and non-null
/// @param ext must be valid and non-null
/// @return the new head of the list
template <typename Node, Node * Node::* prev, Node * Node::* next>
[[nodiscard]] constexpr Node * link_back(Node * ASH_RESTRICT head,
                                         Node * ASH_RESTRICT ext)
{
  link<Node, prev, next>(head, ext);
  return head;
}

/// @brief Attach List `ext` to the front of List `head`, using `head` as anchor
///
/// @param head must be valid and non-null
/// @param ext must be valid and non-null
/// @return the new head of the list
template <typename Node, Node * Node::* prev, Node * Node::* next>
[[nodiscard]] constexpr Node * link_front(Node * ASH_RESTRICT head,
                                          Node * ASH_RESTRICT ext)
{
  link<Node, prev, next>(ext, head);
  return ext;
}

}    // namespace list
}    // namespace intr

/// @brief A non-owning intrusive doubly circularly linked list. This is backed
/// by an external allocator.
/// @tparam N node type
/// @tparam prev previous element getter
/// @tparam next next element getter
template <typename N, N * N::* prev = &N::prev, N * N::* next = &N::next>
struct [[nodiscard]] List
{
  typedef N Node;

  static constexpr Node * Node::* PREV = prev;

  static constexpr Node * Node::* NEXT = next;

  struct Iter
  {
    Node * iter_        = nullptr;
    Node * cursor_      = nullptr;
    bool   past_cursor_ = true;

    constexpr Node & operator*() const
    {
      return *iter_;
    }

    constexpr Iter & operator++()
    {
      iter_        = iter_->*next;
      past_cursor_ = true;
      return *this;
    }

    constexpr bool operator!=(IterEnd) const
    {
      bool const finished = (past_cursor_ & iter_ == cursor_);
      return !finished;
    }

    constexpr bool operator==(IterEnd) const
    {
      return !this->operator!=(iter_end);
    }
  };

  struct RevIter
  {
    Node * iter_        = nullptr;
    Node * cursor_      = nullptr;
    bool   past_cursor_ = true;

    constexpr Node & operator*() const
    {
      return *(iter_->*prev);
    }

    constexpr RevIter & operator++()
    {
      iter_        = iter_->*prev;
      past_cursor_ = true;
      return *this;
    }

    constexpr bool operator!=(IterEnd) const
    {
      bool const finished = (past_cursor_ & iter_ == cursor_);
      return !finished;
    }

    constexpr bool operator==(IterEnd) const
    {
      return !this->operator!=(iter_end);
    }
  };

  struct View
  {
    Node * head_ = nullptr;

    constexpr auto begin() const
    {
      return Iter{
        .iter_ = head_, .cursor_ = head_, .past_cursor_ = (head_ == nullptr)};
    }

    constexpr auto end() const
    {
      return iter_end;
    }
  };

  struct RevView
  {
    Node * head_ = nullptr;

    constexpr auto begin() const
    {
      return RevIter{
        .iter_ = head_, .cursor_ = head_, .past_cursor_ = (head_ == nullptr)};
    }

    constexpr auto end() const
    {
      return iter_end;
    }
  };

  Node * head_ = nullptr;

  constexpr List() = default;

  explicit constexpr List(Node * head) : head_{head}
  {
  }

  constexpr List(List const &) = delete;

  constexpr List(List && other) : head_{other.head_}
  {
    other.head_ = nullptr;
  }

  constexpr List & operator=(List const &) = delete;

  constexpr List & operator=(List && other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }

    head_       = other.head_;
    other.head_ = nullptr;
    return *this;
  }

  constexpr ~List() = default;

  constexpr void leak()
  {
    head_ = nullptr;
  }

  constexpr bool is_empty() const
  {
    return head_ == nullptr;
  }

  [[nodiscard]] constexpr Node * head() const
  {
    return head_;
  }

  [[nodiscard]] constexpr Node * tail() const
  {
    if (head_ == nullptr) [[unlikely]]
    {
      return nullptr;
    }

    return head_->*prev;
  }

  [[nodiscard]] constexpr Node * pop_front()
  {
    if (head_ == nullptr) [[unlikely]]
    {
      return nullptr;
    }

    Node * node = intr::list::unlink_front<Node, prev, next>(head_);
    node->*prev = nullptr;
    node->*next = nullptr;

    return node;
  }

  [[nodiscard]] constexpr Node * pop_back()
  {
    if (head_ == nullptr) [[unlikely]]
    {
      return nullptr;
    }

    Node * node = intr::list::unlink_back<Node, prev, next>(head_);
    node->*prev = nullptr;
    node->*next = nullptr;

    return node;
  }

  static constexpr void unlink_at(Node * node)
  {
    intr::list::unlink<Node, prev, next>(node);
    node->*prev = nullptr;
    node->*next = nullptr;
  }

  /// @param node non-null node to push
  constexpr void push_front(Node * node)
  {
    node->*next = node;
    node->*prev = node;

    if (head_ == nullptr) [[unlikely]]
    {
      head_ = node;
      return;
    }

    head_ = intr::list::link_front<Node, prev, next>(head_, node);
  }

  /// @param node non-null node to push
  constexpr void push_back(Node * node)
  {
    node->*next = node;
    node->*prev = node;

    if (head_ == nullptr) [[unlikely]]
    {
      head_ = node;
      return;
    }

    head_ = intr::list::link_back<Node, prev, next>(head_, node);
  }

  constexpr void extend_front(List list)
  {
    if (list.head_ == nullptr) [[unlikely]]
    {
      return;
    }

    head_ = intr::list::link_front<Node, prev, next>(head_, list.head_);

    list.leak();
  }

  constexpr void extend_back(List list)
  {
    if (list.head_ == nullptr) [[unlikely]]
    {
      return;
    }

    head_ = intr::list::link_back<Node, prev, next>(head_, list.head_);

    list.leak();
  }

  constexpr auto view() const
  {
    return View{.head_ = head_};
  }

  constexpr auto rev_view() const
  {
    return RevView{.head_ = head_};
  }

  constexpr auto begin() const
  {
    return view().begin();
  }

  constexpr auto end() const
  {
    return view().end();
  }

  constexpr auto rbegin() const
  {
    return rev_view().begin();
  }

  constexpr auto rend() const
  {
    return rev_view().end();
  }
};

template <typename N, N * N::* prev, N * N::* next>
struct IsTriviallyRelocatable<List<N, prev, next>>
{
  static constexpr bool value = true;
};

}    // namespace ash
