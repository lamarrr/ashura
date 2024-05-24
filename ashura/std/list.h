#pragma once
#include "ashura/std/types.h"

namespace ash
{

/// @brief Circular Doubly-Linked list node.
template <typename T>
struct ListNode
{
  ListNode<T> *next = nullptr;
  ListNode<T> *prev = nullptr;
  T            data = {};
};

/// @brief Circular Forward Doubly-Linked list.
/// if head is non-null. head->next and head->prev are always non-null.
///
/// @warning only use for scenarios where O(1) random insertion and/or removal
/// is a must.
template <typename T>
struct List
{
  ListNode<T> *head = nullptr;

  static constexpr ListNode<T> *remove_node_link(ListNode<T> *&node)
  {
    if (node == nullptr)
    {
      return nullptr;
    }

    ListNode<T> *out = node;

    // 1 element
    if (node->next == node->prev)
    {
      node = nullptr;
      return out;
    }

    node->next->prev = node->prev;
    node->prev->next = node->next;
    node             = node->next;

    // create 1 element node
    out->next = out;
    out->prev = out;
    return out;
  }

  constexpr ListNode<T> *pop_front()
  {
    return remove_node_link(head);
  }

  constexpr ListNode<T> *pop_back()
  {
    return head == nullptr ? nullptr : remove_node_link(head->prev);
  }

  static constexpr void extend_at_node(ListNode<T> *&node, List<T> list)
  {
    if (list.head == nullptr)
    {
      return;
    }

    if (node == nullptr)
    {
      node = list.head;
      return;
    }

    // adjust, such that the element at node is shifted to position +
    // 1, and replaced with the new list's head.
    ListNode<T> *const node_head = node;
    ListNode<T> *const node_tail = node->prev;
    ListNode<T> *const list_head = list.head;
    ListNode<T> *const list_tail = list.head->prev;
    list_head->prev              = node_head->prev;
    list_tail->next              = node_head;
    node_head->prev              = list_tail;
    node_tail->next              = list_head;
    node                         = list_head;
    return;
  }

  constexpr void extend_front(List<T> list)
  {
    extend_at_node(head, list);
  }

  constexpr void extend_back(List<T> list)
  {
    if (head == nullptr)
    {
      head = list.head;
      return;
    }
    extend_at_node(head->prev, list);
  }

  constexpr bool is_empty() const
  {
    return head == nullptr;
  }
};

}        // namespace ash
