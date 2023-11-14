//
// Created by nikita on 23.08.23.
//

#include "Person.h"
#include "Globals.h"
#include "Time.h"

int64_t Person::get_balance() const noexcept {
  return balance;
}
bool Person::shift_balance(int64_t delta) noexcept {
  if (balance + delta < 0) {
    return false;
  }
  balance += delta;
  return true;
}
bool Person::can_farm() noexcept {
  return time() - last_farm >= farm_time/* && balance < MINIMUM_BIT*/;
}
Person::Person(int64_t bal, int last_f, int64_t bt, int bot_num) noexcept
    : balance(bal), own_bit(bt), last_farm(last_f), bot_count(static_cast<uchar>(bot_num)) {}

int Person::get_last_farm() const noexcept {
  return last_farm;
}
void Person::update_farm() noexcept {
  if (can_farm()) {
    last_farm = time();
  }
}
int64_t Person::getBit() const noexcept {
  return own_bit;
}
bool Person::setBit(int64_t val) noexcept {
  if (val < MINIMUM_BIT) {
    return false;
  }
  own_bit = val;
  return true;
}

int Person::getBots() const noexcept {
  return static_cast<int>(bot_count);
}

bool Person::setBots(int bots) noexcept {
  if (bots >= 0 && bots <= max_bots) {
    bot_count = static_cast<uchar>(bots);
    return true;
  }
  return false;
}
