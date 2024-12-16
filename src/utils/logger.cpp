#include "utils/logger.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <chrono>
#include <memory>
#include <vector>

using namespace std;

void init_logger() {
  const size_t sink_size = 8192;
  spdlog::level::level_enum console_lvl = spdlog::level::warn;
  spdlog::level::level_enum file_lvl = spdlog::level::info;
  spdlog::init_thread_pool(sink_size, 1);
  auto stdout_sink = make_shared<spdlog::sinks::stdout_color_sink_mt>();
  stdout_sink->set_level(console_lvl);
  auto file_sink = make_shared<spdlog::sinks::basic_file_sink_mt>("log.txt");
  file_sink->set_level(file_lvl);
  file_sink->set_level(file_lvl);
  vector<spdlog::sink_ptr> sinks({stdout_sink, file_sink});
  auto logger = make_shared<spdlog::async_logger>(
      "sdktest", sinks.begin(), sinks.end(), spdlog::thread_pool(),
      spdlog::async_overflow_policy::block);
  logger->set_level(spdlog::level::debug);
  spdlog::register_logger(logger);
  spdlog::set_default_logger(logger);
  spdlog::flush_every(chrono::seconds(5));
}
