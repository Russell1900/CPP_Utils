#pragma once
#include "spdlog/spdlog.h"

#define CHECK_CRITICAL(ret, msg, ...)                                          \
  if (ret != 0) {                                                              \
    spdlog::critical(msg, "\nretcode: {}", ##__VA_ARGS__, ret);                \
    exit(1);                                                                   \
  }
#define CHECK_ERROR(ret, msg, ...)                                             \
  if (ret != 0) {                                                              \
    spdlog::error(msg, "\nretcode: {}", ##__VA_ARGS__, ret);                   \
  }
