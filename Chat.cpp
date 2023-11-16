//
// Created by nikita on 14.11.23.
//

#include "Chat.h"

std::ostream& operator<<(std::ostream& out, const Chat& chat) {
  out << chat.bit << ' ' << chat.bot_count;
  return out;
}
std::istream& operator>>(std::istream& in, Chat& chat) {
  in >> chat.bit >> chat.bot_count;
  return in;
}
int64_t Chat::getBit() const noexcept {
  return bit;
}
uchar Chat::getBots() const noexcept {
  return bot_count;
}

bool Chat::setBots(int bots) noexcept {
  if (bots >= 0 && bots <= max_bots) {
    bot_count = static_cast<uchar>(bots);
    return true;
  }
  return false;
}
bool Chat::setBit(int64_t val) noexcept {
  if (val < MINIMUM_BIT) {
    return false;
  }
  bit = val;
  return true;
}
