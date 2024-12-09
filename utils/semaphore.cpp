#include "utils/semaphore.h"

using namespace std;

Semaphore::Semaphore(size_t capacity) : capacity_(capacity) {}

void Semaphore::acquire() {
  unique_lock<mutex> ul(lck_);
  cv_.wait(ul, [this] { return this->capacity_ > 0; });
  --capacity_;
}

void Semaphore::release() {
  ++capacity_;
  cv_.notify_one();
}
