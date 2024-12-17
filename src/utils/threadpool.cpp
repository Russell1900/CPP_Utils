#include "threadpool.h"
#include "spdlog/spdlog.h"

ThreadPool::ThreadPool(unsigned int worker_num)
    : _max_work_num(thread::hardware_concurrency()), _shutdown(false),
      _shutdown_now(false) {
  if (worker_num > _max_work_num) {
    spdlog::warn("work num({}) > max hardware thread num({}), set to {}",
                 worker_num, _max_work_num, _max_work_num);
    _worker_num.store(_max_work_num);
  } else if (worker_num == 0) {
    spdlog::warn("work num is {}, set to {}", worker_num, _max_work_num);
    _worker_num.store(_max_work_num);
  } else {
    _worker_num.store(worker_num);
  }

  for (int i = 0; i < _worker_num.load(); ++i) {
    _worker_pool.emplace_back(thread(&ThreadPool::_worker, this));
  }
  _status.store(ThreadPool::Status::RUNNING);
  _total_task_num.store(0);
  _finished_task_num.store(0);
  spdlog::info("thread pool inited, worker number: {}.", _worker_num.load());
}

void ThreadPool::_worker() {
  try {
    for (;;) {
      function<void()> task = nullptr;
      {
        unique_lock<mutex> lck(_mtx);
        _cv.wait(lck, [this]() {
          return this->_shutdown_now || this->_shutdown ||
                 !this->_task_pool.empty();
        });
        if (_shutdown_now) {
          break;
        }

        if (!_task_pool.empty()) {
          task = ::std::move(_task_pool.front());
          _task_pool.pop();
        } else {
          if (_shutdown) {
            break;
          }
        }
      }
      if (task) {
        task();
        _finished_task_num++;
      }
    }
  } catch (const exception &e) {
    spdlog::error("error in thread: {}", e.what());
  }
}

bool ThreadPool::shutdown(bool now) {
  if (_status == ThreadPool::Status::CLOSED) {
    spdlog::error("thread pool is already closed.");
    return false;
  }
  if (now) {
    _shutdown_now = true;
  } else {
    _shutdown = true;
  }
  _status.store(ThreadPool::Status::SHUTING_DOWN);
  _cv.notify_all();
  return true;
}

unsigned int ThreadPool::join() {
  if (_status == ThreadPool::Status::RUNNING) {
    spdlog::error("need shutdown threadpool first");
    return 0;
  }
  if (_status == ThreadPool::Status::CLOSED) {
    spdlog::error("threadpool is already closed");
    return 0;
  }
  for (auto &worker : _worker_pool) {
    worker.join();
  }
  return _finished_task_num.load();
}
