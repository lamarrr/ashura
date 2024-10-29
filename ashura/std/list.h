/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/types.h"

namespace ash
{

/// @brief Circular Doubly-Linked list node.
/// head->next and head->prev are always non-null.
///
/// always construct with operator new.
///
/// @warning only use for scenarios where O(1) random insertion and/or removal
/// is a must. ListNode requires stable addressing, must not be relocated once
/// constructed.
///
template <typename T>
struct ListNode : Pin<>
{
  ListNode<T> *next = this;
  ListNode<T> *prev = this;
  T            data = {};

  void isolate()
  {
    next = this;
    prev = this;
  }

  [[nodiscard]] constexpr bool is_linked() const
  {
    return next != nullptr && prev != nullptr;
  }

  [[nodiscard]] constexpr bool is_isolated() const
  {
    return next == this && prev == this;
  }
};

namespace list
{

/// @brief
/// @tparam T
/// @param node must be valid and non-null
template <typename T>
static constexpr void unlink_node(ListNode<T> *node)
{
  // detach from siblings
  node->next->prev = node->prev;
  node->prev->next = node->next;
  // create 1 element node
  node->next = node;
  node->prev = node;
}

/// @brief
/// @tparam T
/// @param head must be valid and non-null, set to null if empty
/// @return popped element or null
template <typename T>
[[nodiscard]] constexpr ListNode<T> *pop_front(ListNode<T> *&head)
{
  ListNode<T> *out      = head;
  ListNode<T> *new_head = (head->next == head) ? nullptr : head->next;
  unlink_node(out);
  head = new_head;
  return out;
}

/// @brief
/// @tparam T
/// @param head must be valid and non-null, set to null if empty
/// @return
template <typename T>
[[nodiscard]] constexpr ListNode<T> *pop_back(ListNode<T> *&head)
{
  ListNode<T> *out      = head->prev;
  ListNode<T> *new_head = (head->prev == head) ? nullptr : head;
  unlink_node(out);
  head = new_head;
  return out;
}

///
/// @brief
///
/// @tparam T
/// @param node must be valid and non-null
/// @param ext must be valid and non-null
///
template <typename T>
constexpr void attach(ListNode<T> *node, ListNode<T> *ext)
{
  ListNode<T> *node_head = node;
  ListNode<T> *node_tail = node->prev;
  ListNode<T> *ext_head  = ext;
  ListNode<T> *ext_tail  = ext->prev;
  ext_head->prev         = node_tail;
  ext_tail->next         = node_head;
  node_head->prev        = ext_tail;
  node_tail->next        = ext_head;
}

///
/// @brief
///
/// @tparam T
/// @param head must be valid and non-null
/// @param ext must be valid and non-null
///
template <typename T>
[[nodiscard]] constexpr ListNode<T> *push_back(ListNode<T> *head,
                                               ListNode<T> *ext)
{
  attach(head, ext);
  return head;
}

///
/// @brief
///
/// @tparam T
/// @param head must be valid and non-null
/// @param ext must be valid and non-null
///
template <typename T>
[[nodiscard]] constexpr ListNode<T> *push_front(ListNode<T> *head,
                                                ListNode<T> *ext)
{
  attach(ext, head);
  return ext;
}

}        // namespace list

template <typename T>
struct List
{
  ListNode<T> *head = nullptr;

  constexpr bool is_empty() const
  {
    return head == nullptr;
  }

  [[nodiscard]] constexpr ListNode<T> *tail() const
  {
    if (head == nullptr) [[unlikely]]
    {
      return nullptr;
    }

    return head->prev;
  }

  [[nodiscard]] constexpr ListNode<T> *pop_front()
  {
    if (head == nullptr) [[unlikely]]
    {
      return nullptr;
    }

    return list::pop_front(head);
  }

  [[nodiscard]] constexpr ListNode<T> *pop_back()
  {
    if (head == nullptr) [[unlikely]]
    {
      return nullptr;
    }

    return list::pop_back(head);
  }

  constexpr void push_front(ListNode<T> *ext)
  {
    if (head == nullptr) [[unlikely]]
    {
      head = ext;
      return;
    }

    head = list::push_front(head, ext);
  }

  constexpr void push_back(ListNode<T> *ext)
  {
    if (head == nullptr) [[unlikely]]
    {
      head = ext;
      return;
    }

    head = list::push_back(head, ext);
  }
};

}        // namespace ash
