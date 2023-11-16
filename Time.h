//
// Created by nikita on 23.08.23.
//

#ifndef CASINO_TIME_H
#define CASINO_TIME_H

#include <chrono>

struct original_t {};

int time() noexcept;

std::time_t time(original_t) noexcept;

#endif//CASINO_TIME_H
