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
  return time() - last_farm >= farm_time /* && balance < MINIMUM_BIT*/;
}

void Person::update_farm() noexcept {
  if (can_farm()) {
    last_farm = time();
  }
}

bool Person::add_bit() noexcept {
  if (active_bits < MAX_ACTIVE_BITS) {
    ++active_bits;
    return true;
  }
  return false;
}
void Person::close_bit() noexcept {
  if (active_bits > 0) {
    --active_bits;
  }
}
bool Person::has_active_bit() const noexcept {
  return active_bits;
}

std::ostream& operator<<(std::ostream& out, const Person& person) {
  out << person.balance << ' ' << person.last_farm;
  return out;
}

std::istream& operator>>(std::istream& in, Person& person) {
  in >> person.balance >> person.last_farm;
  return in;
}
