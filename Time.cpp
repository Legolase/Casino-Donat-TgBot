//
// Created by nikita on 23.08.23.
//
#include "Time.h"

int time() noexcept {
    return time(original_t{});
}

std::time_t time(original_t) noexcept {
  return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) + 3 * 3600;
}
