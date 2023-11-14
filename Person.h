//
// Created by nikita on 23.08.23.
//

#ifndef CASINO_PERSON_H
#define CASINO_PERSON_H

#include <cstdint>
#include "Globals.h"
#include <set>

struct Person {
  static constexpr int64_t MINIMUM_BIT = 20;
  static constexpr int64_t FARM_AMOUNT = MINIMUM_BIT * 3;
  static constexpr int DEFAULT_BOTS = 5;

  Person() noexcept = default;
  explicit Person(int64_t balance, int last_f, int64_t bt, int bot_num) noexcept;

  int64_t get_balance() const noexcept;
  bool shift_balance(int64_t delta) noexcept;
  bool can_farm() noexcept;
  void update_farm() noexcept;
  int get_last_farm() const noexcept;
  int64_t getBit() const noexcept;
  bool setBit(int64_t val) noexcept;
  int getBots() const noexcept;
  bool setBots(int bots) noexcept;

  std::set<int64_t> in_game;
private:
  int64_t balance{FARM_AMOUNT};
  int64_t own_bit{MINIMUM_BIT};
  int last_farm{0};
  uchar bot_count{DEFAULT_BOTS};
};

#endif // CASINO_PERSON_H
