//
// Created by nikita on 23.08.23.
//
#include "Time.h"
#include <chrono>

int time() noexcept {
    return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}
