#pragma once
#include "ashura/std/types.h"

namespace ash
{

/// @brief only use for scenarios where O(1) insertion and/or removal is a must.
template <typename T>
struct ListNode
{
  ListNode<T> *next = nullptr;
  T            v    = {};
};

template <typename T>
constexpr usize count(ListNode<T> const *start);

template <typename T>
constexpr void reverse(ListNode<T> *head);

template <typename T>
constexpr bool is_cyclic(ListNode<T> const &start);

template <typename T>
constexpr ListNode<T> *pop(ListNode<T> &node);

template <typename T>
constexpr void push(ListNode<T> &head, ListNode<T> *node);

}        // namespace ash
