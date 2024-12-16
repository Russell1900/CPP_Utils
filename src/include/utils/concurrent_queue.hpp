#pragma once
#include <condition_variable>
#include <deque>
#include <mutex>

template <typename T> class ConcurrentQueue {
protected:
  std::deque<T> q_;
  size_t capacity_;
  std::condition_variable cv_out_;
  std::condition_variable cv_in_;
  std::mutex io_lck_;

public:
  ConcurrentQueue(size_t capacity);
  size_t size();
  size_t capacity();
  void push(const T &element);
  void push(T &&element);
  T pop();
};

template <typename T>
ConcurrentQueue<T>::ConcurrentQueue(size_t capacity) : capacity_(capacity) {}

template <typename T> size_t ConcurrentQueue<T>::size() { return q_.size(); }

template <typename T> size_t ConcurrentQueue<T>::capacity() {
  return capacity_;
}

template <typename T> void ConcurrentQueue<T>::push(const T &element) {

  std::unique_lock<std::mutex> ul(io_lck_);
  cv_in_.wait(ul, [this] { return q_.size() < capacity_; });
  q_.emplace_back(element);
  cv_out_.notify_one();
}

template <typename T> void ConcurrentQueue<T>::push(T &&element) {
  std::unique_lock<std::mutex> ul(io_lck_);
  cv_in_.wait(ul, [this] { return q_.size() < capacity_; });
  q_.emplace_back(element);
  cv_out_.notify_one();
}

template <typename T> T ConcurrentQueue<T>::pop() {
  std::unique_lock<std::mutex> ul(io_lck_);
  cv_out_.wait(ul, [this] { return q_.size() > 0; });
  T tmp = std::move(q_.front());
  q_.pop_front();
  cv_in_.notify_one();
  return tmp;
}
