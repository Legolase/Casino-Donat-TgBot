//
// Created by nikita on 23.08.23.
//

#ifndef CASINO_PERSON_H
#define CASINO_PERSON_H

#include <cstdint>
#include "Globals.h"
#include <set>
#include <unordered_set>

struct Person {
  Person() noexcept = default;
  Person(Person const&) = delete;
  explicit Person(int64_t balance, int last_f) noexcept;

  int64_t get_balance() const noexcept;
  bool shift_balance(int64_t delta) noexcept;

  bool can_farm() noexcept;
  void update_farm() noexcept;
  int get_last_farm() const noexcept;

  bool add_bit() noexcept;
  bool has_active_bit() const noexcept;
  void close_bit() noexcept;
private:
  int64_t balance{FARM_AMOUNT};
  int last_farm{0};
  uchar active_bits{0};
//  int64_t own_bit{MINIMUM_BIT}; // ok
//  uchar bot_count{DEFAULT_BOTS}; // ok
};

#endif // CASINO_PERSON_H
