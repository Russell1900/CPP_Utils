#include "utils/recorder.h"
#include "spdlog/spdlog.h"
#include <iostream>
#include <thread>
#ifdef WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

using json = nlohmann::json;
using namespace std;

// class RealTimeRecorderManager
RealTimeRecorderManager::RealTimeRecorderManager(const string &filepath)
    : joined_(false), started_(false), filepath_(filepath) {}

void RealTimeRecorderManager::start(size_t interval) {
  if (!started_) {
    interval_ = interval > 0 ? interval : 5000;
    started_ = true;
    task_ = thread(&RealTimeRecorderManager::task, this);
  }
}

RealTimeRecorderManager::~RealTimeRecorderManager() {}

void RealTimeRecorderManager::join() {
  json res;
  if (started_ && !joined_) {
    joined_ = true;
    task_.join();
    for (auto &recorder : recorders_) {
      recorder->get_result(res);
    }
    ofstream ofs(filepath_);
    if (ofs.is_open()) {
      res.dump(2);
      ofs << res;
      ofs.flush();
      ofs.close();
    } else {
      spdlog::error("result file open failed: {}", filepath_);
    }
  }
}

void RealTimeRecorderManager::task() {
  while (!joined_) {
    for (auto &recorder : recorders_) {
      recorder->tick();
    }
#ifdef WIN32
    Sleep(interval_);
#else
    usleep(interval_ * 1000);
#endif
  }
}

void RealTimeRecorderManager::add_recorder(RealTimeRecorder *recorder) {
  recorders_.emplace_back(recorder);
}

// class RealTimeRecorder
RealTimeRecorder::RealTimeRecorder(const string &name) : name_(name) {}
RealTimeRecorder::~RealTimeRecorder() {}

// class TpsMonitor
TpsMonitor::TpsMonitor(const string &name, bool b_tick)
    : RealTimeRecorder(name), b_tick_(b_tick), op_num(0), pre_num(0),
      pre_tp(std::chrono::high_resolution_clock::now()) {}

TpsMonitor::TpsMonitor(const TpsMonitor &rval) : RealTimeRecorder(rval.name_) {
  op_num = rval.op_num.load();
  pre_num = rval.pre_num;
  pre_tp = rval.pre_tp;
}

void TpsMonitor::tick() {
  if (!b_tick_)
    return;
  std::chrono::high_resolution_clock::time_point cur_tp =
      std::chrono::high_resolution_clock::now();
  size_t cur_op_num = op_num;
  double duration = static_cast<double>(
      chrono::duration_cast<chrono::milliseconds>(cur_tp - pre_tp).count());
  duration /= 1000;
  double tps = (cur_op_num - pre_num) / duration;
  float new_tps = int(tps * 10 + 0.5) / 10.0;
  spdlog::warn("duration: {}, opnum: {}", duration, cur_op_num);
  spdlog::warn("TPS: {}", new_tps);
  pre_tp = cur_tp;
  pre_num = cur_op_num;
}

void TpsMonitor::get_result(json &res) { res[name_] = op_num.load(); }

TpsMonitor &TpsMonitor::operator++() {
  ++op_num;
  return *this;
}

TpsMonitor TpsMonitor::operator++(int) {
  TpsMonitor tmp(*this);
  ++op_num;
  return tmp;
}
