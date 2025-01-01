#pragma once

#include "memory.h"

template <typename T>
class node_t : T
{
  node_t *next;
  node_t *prev;

public:
  template <typename... Args>
  node_t(Args... args) : T(args...), next{nullptr}, prev{nullptr}
  {
  }
};

template <typename T>
class list_t
{
  node_t<T> *head;
  node_t<T> *tail;

public:
  list_t() : head{nullptr}, tail{nullptr} {}

  void push_front(node_t<T> *node) {}

  template <typename... Args>
  void push_front(Args... args)
  {
    auto node = new node_t<T>{args...};

    head->next = node;
    node->prev = head;
  }
};
