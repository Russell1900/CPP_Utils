/* c++11 Semaphore */
#pragma once
#include <atomic>
#include <condition_variable>
#include <mutex>

class Semaphore {
protected:
  std::mutex lck_;
  std::condition_variable cv_;

public:
  Semaphore(size_t capacity);
  std::atomic<size_t> capacity_;
  void acquire();
  void release();
};
