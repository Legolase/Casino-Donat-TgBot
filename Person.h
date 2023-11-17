//
// Created by nikita on 23.08.23.
//

#ifndef CASINO_PERSON_H
#define CASINO_PERSON_H

#include "Globals.h"
#include <cstdint>
#include <istream>
#include <ostream>
#include <set>
#include <unordered_set>

struct Person {
  Person() noexcept = default;

  int64_t get_balance() const noexcept;
  bool shift_balance(int64_t delta) noexcept;

  bool can_farm() const noexcept;
  void update_farm() noexcept;

  bool add_bit() noexcept;
  bool has_active_bit() const noexcept;
  void close_bit() noexcept;

  friend std::ostream& operator<<(std::ostream& out, Person const& person);
  friend std::istream& operator>>(std::istream& in, Person& person);

private:
  int64_t balance{MAX_FARM_AMOUNT};
  int last_farm{0};
  uchar active_bits{0};
};

#endif // CASINO_PERSON_H
