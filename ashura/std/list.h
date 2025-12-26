/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/types.h"

namespace ash
{

namespace intr
{
namespace dlist
{

/// @brief Clear the links of Node `node`, producing an isolated node
template <typename Node, Node * Node::* PREV, Node * Node::* NEXT>
constexpr Node * clear_links(Node * node)
{
  node->*NEXT = nullptr;
  node->*PREV = nullptr;
  return node;
}

/// @brief Unlink Node `head` from the List, producing an isolated node
/// @param head must be valid and non-null
/// @return popped list, never null
template <typename Node, Node * Node::* PREV, Node * Node::* NEXT>
constexpr void unlink(Node * at)
{
  // detach from siblings
  at->*NEXT->*PREV = at->*PREV;
  at->*PREV->*NEXT = at->*NEXT;
}

/// @brief Remove from the front of the list, producing an isolated node
/// @param head must be valid and non-null, set to null if empty
/// @return unlinked element or null
template <typename Node, Node * Node::* PREV, Node * Node::* NEXT>
[[nodiscard]] constexpr Node * unlink_front(Node * head)
{
  auto * out = head->*NEXT;
  unlink<Node, PREV, NEXT>(out);
  return out;
}

/// @brief Remove from the back of the list, producing an isolated node
/// @param head must be valid and non-null, set to null if empty
/// @return unlinked element or null
template <typename Node, Node * Node::* PREV, Node * Node::* NEXT>
[[nodiscard]] constexpr Node * unlink_back(Node * head)
{
  auto * out = head->*PREV;
  unlink<Node, PREV, NEXT>(out);
  return out;
}

template <typename Node, Node * Node::* PREV, Node * Node::* NEXT>
constexpr bool is_empty(Node * ASH_RESTRICT head)
{
  return head->*NEXT == head;
}

/// @brief Attach Node `node` to the end of List `head`
///
/// @param head must be valid and non-null
/// @param ext must be valid and non-null
///
template <typename Node, Node * Node::* PREV, Node * Node::* NEXT>
constexpr void push_back(Node * ASH_RESTRICT head, Node * ASH_RESTRICT node)
{
  node->*PREV        = head->*PREV;
  node->*NEXT        = head;
  head->*PREV->*NEXT = node;
  head->*PREV        = node;
}

/// @brief Attach List `ext` to the front of List `head`
///
/// @param head must be valid and non-null
/// @param ext must be valid and non-null
///
template <typename Node, Node * Node::* PREV, Node * Node::* NEXT>
constexpr void push_front(Node * ASH_RESTRICT head, Node * ASH_RESTRICT node)
{
  node->*NEXT        = head->*NEXT;
  node->*PREV        = head;
  head->*NEXT->*PREV = node;
  head->*NEXT        = node;
}

template <typename Node, Node * Node::* PREV, Node * Node::* NEXT>
constexpr Node * pop_back(Node * ASH_RESTRICT head)
{
  if (is_empty<Node, PREV, NEXT>(head)) [[unlikely]]
  {
    return nullptr;
  }

  auto * out = unlink_back<Node, PREV, NEXT>(head);
  return clear_links<Node, PREV, NEXT>(out);
}

template <typename Node, Node * Node::* PREV, Node * Node::* NEXT>
constexpr Node * pop_front(Node * ASH_RESTRICT head)
{
  if (is_empty<Node, PREV, NEXT>(head)) [[unlikely]]
  {
    return nullptr;
  }

  auto * out = unlink_front<Node, PREV, NEXT>(head);
  return clear_links<Node, PREV, NEXT>(out);
}

template <typename Node, Node * Node::* PREV, Node * Node::* NEXT>
constexpr void pop_at(Node * ASH_RESTRICT node)
{
  unlink<Node, PREV, NEXT>(node);
  clear_links<Node, PREV, NEXT>(node);
}

}    // namespace dlist
}    // namespace intr

template <typename N, N * N::* prev = &N::prev, N * N::* next = &N::next>
struct ListIter
{
  typedef N Node;

  static constexpr Node * Node::* PREV = prev;

  static constexpr Node * Node::* NEXT = next;

  Node * iter_ = nullptr;
  Node * head_ = nullptr;

  constexpr Node & operator*() const
  {
    return *iter_;
  }

  constexpr ListIter & operator++()
  {
    iter_ = iter_->*NEXT;
    return *this;
  }

  constexpr bool operator!=(IterEnd) const
  {
    return iter_ != head_;
  }

  constexpr bool operator==(IterEnd) const
  {
    return !this->operator!=(iter_end);
  }
};

template <typename N, N * N::* prev = &N::prev, N * N::* next = &N::next>
struct RevListIter
{
  typedef N Node;

  static constexpr Node * Node::* PREV = prev;

  static constexpr Node * Node::* NEXT = next;

  Node * iter_ = nullptr;
  Node * head_ = nullptr;

  constexpr Node & operator*() const
  {
    return *iter_;
  }

  constexpr RevListIter & operator++()
  {
    iter_ = iter_->*PREV;
    return *this;
  }

  constexpr bool operator!=(IterEnd) const
  {
    return iter_ != head_;
  }

  constexpr bool operator==(IterEnd) const
  {
    return !this->operator!=(iter_end);
  }
};

template <typename N, N * N::* prev = &N::prev, N * N::* next = &N::next>
struct RevListView
{
  typedef N Node;

  static constexpr Node * Node::* PREV = prev;

  static constexpr Node * Node::* NEXT = next;

  Node * head_;

  explicit constexpr RevListView(Node * head) : head_{head}
  {
  }

  constexpr bool is_empty() const
  {
    return (head_->*NEXT == head_);
  }

  void clear()
  {
    head_->*NEXT = head_;
    head_->*PREV = head_;
  }

  [[nodiscard]] constexpr Node * head() const
  {
    return head_;
  }

  constexpr auto begin() const
  {
    return RevListIter{.iter_ = head_->*PREV, .head_ = head_};
  }

  constexpr auto end() const
  {
    return iter_end;
  }
};

/// @brief A non-owning intrusive doubly circularly linked list. This is backed
/// by an external allocator.
/// @tparam N node type
/// @tparam prev previous element getter
/// @tparam next next element getter
template <typename N, N * N::* prev = &N::prev, N * N::* next = &N::next>
struct [[nodiscard]] ListView
{
  typedef N Node;

  static constexpr Node * Node::* PREV = prev;

  static constexpr Node * Node::* NEXT = next;

  Node * head_;

  explicit constexpr ListView(Node * head) : head_{head}
  {
  }

  constexpr bool is_empty() const
  {
    return (head_->*NEXT == head_);
  }

  void clear()
  {
    head_->*NEXT = head_;
    head_->*PREV = head_;
  }

  [[nodiscard]] constexpr Node * head() const
  {
    return head_;
  }

  [[nodiscard]] constexpr Node * pop_front()
  {
    return intr::dlist::pop_front<Node, PREV, NEXT>(head_);
  }

  [[nodiscard]] constexpr Node * pop_back()
  {
    return intr::dlist::pop_back<Node, PREV, NEXT>(head_);
  }

  static constexpr void pop_at(Node * node)
  {
    intr::dlist::pop_at<Node, PREV, NEXT>(node);
  }

  /// @param node non-null node to push
  constexpr void push_front(Node * ASH_RESTRICT node)
  {
    intr::dlist::push_front<Node, PREV, NEXT>(head_, node);
  }

  /// @param node non-null node to push
  constexpr void push_back(Node * ASH_RESTRICT node)
  {
    intr::dlist::push_back<Node, PREV, NEXT>(head_, node);
  }

  constexpr auto rev() const
  {
    return RevListView{.head_ = head_};
  }

  constexpr auto begin() const
  {
    return ListIter{.iter_ = head_->*NEXT, .head_ = head_};
  }

  constexpr auto end() const
  {
    return iter_end;
  }

  constexpr auto rbegin() const
  {
    return rev().begin();
  }

  constexpr auto rend() const
  {
    return rev().end();
  }
};

template <typename N, N * N::* prev = &N::prev, N * N::* next = &N::next>
struct List
{
  typedef N Node;

  static constexpr Node * Node::* PREV = prev;

  static constexpr Node * Node::* NEXT = next;

  using Iter    = ListIter<N, PREV, NEXT>;
  using RevIter = RevListIter<N, PREV, NEXT>;
  using View    = ListView<N, PREV, NEXT>;
  using RevView = RevListView<N, PREV, NEXT>;

  union
  {
    N    head_;
    char inactive_;
  };

  constexpr List() : inactive_{0}
  {
    head_.*PREV = &head_;
    head_.*NEXT = &head_;
  }

  constexpr List(List const &)             = delete;
  constexpr List(List &&)                  = delete;
  constexpr List & operator=(List const &) = delete;
  constexpr List & operator=(List &&)      = delete;

  constexpr ~List()
  {
  }

  constexpr bool is_empty() const
  {
    return (head_.*NEXT == &head_);
  }

  void clear()
  {
    head_.*NEXT = &head_;
    head_.*PREV = &head_;
  }

  [[nodiscard]] constexpr auto head()
  {
    return &head_;
  }

  [[nodiscard]] constexpr Node * pop_front()
  {
    return intr::dlist::pop_front<Node, PREV, NEXT>(&head_);
  }

  [[nodiscard]] constexpr Node * pop_back()
  {
    return intr::dlist::pop_back<Node, PREV, NEXT>(&head_);
  }

  static constexpr void pop_at(Node * node)
  {
    intr::dlist::pop_at<Node, PREV, NEXT>(node);
  }

  /// @param node non-null node to push
  constexpr void push_front(Node * ASH_RESTRICT node)
  {
    intr::dlist::push_front<Node, PREV, NEXT>(&head_, node);
  }

  /// @param node non-null node to push
  constexpr void push_back(Node * ASH_RESTRICT node)
  {
    intr::dlist::push_back<Node, PREV, NEXT>(&head_, node);
  }

  constexpr auto view()
  {
    return View{&head_};
  }

  constexpr auto rev_view()
  {
    return RevView{&head_};
  }

  constexpr auto begin()
  {
    return Iter{.iter_ = head_.*NEXT, .head_ = &head_};
  }

  constexpr auto end()
  {
    return iter_end;
  }

  constexpr auto rbegin()
  {
    return RevIter{.iter_ = head_.*PREV, .head_ = &head_};
  }

  constexpr auto rend()
  {
    return iter_end;
  }
};

}    // namespace ash
