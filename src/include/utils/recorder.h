#pragma once
#include "spdlog/spdlog.h"
#include <atomic>
#include <chrono>
#include <fstream>
#include <json.hpp>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class RealTimeRecorder;

// class RealTimeRecorderManager
class RealTimeRecorderManager {
protected:
  std::vector<RealTimeRecorder *> recorders_;
  size_t interval_; // ms
  std::thread task_;
  bool joined_;
  bool started_;
  std::string filepath_;

  void task();

public:
  RealTimeRecorderManager(const std::string &filepath);
  ~RealTimeRecorderManager();
  virtual void start(size_t interval = 5000);
  virtual void join();
  virtual void add_recorder(RealTimeRecorder *recorder);
};

// class RealTimeRecorder
class RealTimeRecorder {
protected:
  friend class RealTimeRecorderManager;

  std::string name_;

  RealTimeRecorder(const std::string &name);
  virtual void tick() = 0;
  virtual void get_result(nlohmann::json &result) = 0;

public:
  virtual ~RealTimeRecorder();
};

// class FileRecorder
template <typename T> class FileRecorder : public RealTimeRecorder {
protected:
  std::string filepath_;
  std::ofstream ofs_;
  std::vector<T> datas_;
  std::mutex lck_;
  bool is_done_;
  bool b_merge_;

  virtual void tick() override {
    std::vector<T> tmp_datas;
    {
      std::unique_lock<std::mutex> uqklck(lck_);
      tmp_datas = datas_;
      datas_.clear();
    }
    for (T &data : tmp_datas) {
      ofs_ << data << std::endl;
    }
  }
  virtual void get_result(nlohmann::json &res) override {
    done_();
    if (!b_merge_)
      return;
    std::ifstream ifs(filepath_);
    T line;
    res[name_] = nlohmann::json::array();
    if (ifs.is_open()) {
      while (ifs >> line) {
        res[name_].push_back(line);
      }
    } else {
      spdlog::error("failed to reopen {}", filepath_);
    }
  }
  virtual void done_() {
    if (!is_done_) {
      is_done_ = true;
      if (ofs_.is_open()) {
        tick();
        ofs_.flush();
        ofs_.close();
      }
    }
  }

public:
  FileRecorder(const std::string &name, const std::string &filepath,
               bool b_merge = true)
      : RealTimeRecorder(name), filepath_(filepath), is_done_(false),
        b_merge_(b_merge) {
    ofs_.open(filepath_);
    if (!ofs_.is_open()) {
      spdlog::critical("fail to open file: {}", filepath_);
      exit(1);
    }
  }
  virtual ~FileRecorder() { done_(); }
  virtual void push_back(T &&data) {
    std::unique_lock<std::mutex> uqlck(lck_);
    datas_.emplace_back(data);
  }
  virtual void push_back(const T &data) {
    std::unique_lock<std::mutex> uqlck(lck_);
    datas_.emplace_back(data);
  }
};

// class TpsMonitro
class TpsMonitor : public RealTimeRecorder {
protected:
  std::atomic<size_t> op_num;

  size_t pre_num;
  std::chrono::high_resolution_clock::time_point pre_tp;
  bool b_tick_;

  virtual void tick() override;
  virtual void get_result(nlohmann::json &res) override;

public:
  TpsMonitor(const std::string &name, bool b_tick = true);
  TpsMonitor(const TpsMonitor &rval);
  TpsMonitor &operator++();
  TpsMonitor operator++(int);
};
