#pragma once
#include "spdlog/spdlog.h"
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
using namespace std;

class ThreadPool {
public:
  typedef enum Status { RUNNING = 0, SHUTING_DOWN, CLOSED } status;

  ThreadPool(unsigned int worker_num);
  ~ThreadPool();

  void shutdown(bool now = false);

  template <typename F, typename... Args>
  shared_ptr<future<typename result_of<F(Args...)>::type>>
  submit(F &&f, Args &&...args) {
    if (_status != ThreadPool::Status::RUNNING) {
      spdlog::error("Fail to add task into threadpool, status: {}",
                    static_cast<int>(_status));
      return nullptr;
    }
    using return_type = typename result_of<F(Args...)>::type;
    auto task = make_shared<packaged_task<return_type()>>(
        bind(forward<F>(f), forward<Args>(args)...));
    {
      unique_lock<mutex> lck(_mtx);
      _task_pool.push([task]() { (*task)(); });
      _total_task_num++;
      _cv.notify_one();
    }
    return make_shared<future<return_type>>(::std::move(task->get_future()));
  }

private:
  atomic<unsigned int> _worker_num;
  unsigned int _max_work_num;
  vector<thread> _worker_pool;
  queue<function<void()>> _task_pool;
  mutex _mtx;
  condition_variable _cv;
  bool _shutdown_now;
  bool _shutdown;
  atomic<ThreadPool::Status> _status;
  atomic<unsigned int> _total_task_num;
  atomic<unsigned int> _finished_task_num;

  void _worker();
  unsigned int join();
};
