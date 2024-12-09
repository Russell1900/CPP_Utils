#pragma once
#include "utils/semaphore.h"
#include <deque>
#include <mutex>

template <typename T> class ConcurrentQueue {
protected:
  std::deque<T> q_;
  size_t capacity_;
  Semaphore smp_;
  std::mutex io_lck;

public:
  ConcurrentQueue(size_t capacity);
  size_t size();
  size_t capacity();
  void push(const T &element);
  void push(T &&element);
  T pop();
};

template <typename T>
ConcurrentQueue<T>::ConcurrentQueue(size_t capacity)
    : capacity_(capacity), smp_(capacity) {}

template <typename T> size_t ConcurrentQueue<T>::size() { return q_.size(); }

template <typename T> size_t ConcurrentQueue<T>::capacity() {
  return capacity_;
}

template <typename T> void ConcurrentQueue<T>::push(const T &element) {
  smp_.acquire();
  std::unique_lock<std::mutex> ul(io_lck);
  q_.push_back(element);
}

template <typename T> void ConcurrentQueue<T>::push(T &&element) {
  smp_.acquire();
  std::unique_lock<std::mutex> ul(io_lck);
  q_.push_back(element);
}

template <typename T> T ConcurrentQueue<T>::pop() {
  std::unique_lock<std::mutex> ul(io_lck);
  T tmp = std::move(q_.front());
  q_.pop_front();
  smp_.release();
  return tmp;
}
