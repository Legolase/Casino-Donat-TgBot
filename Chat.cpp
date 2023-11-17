//
// Created by nikita on 14.11.23.
//

#include "Chat.h"

std::ostream& operator<<(std::ostream& out, const Chat& chat) {
  out << chat.bit << ' ' << static_cast<int>(chat.bot_count) << ' ' << static_cast<int>(chat.color_count);
  return out;
}

std::istream& operator>>(std::istream& in, Chat& chat) {
  int i;
  in >> chat.bit >> i;
  chat.bot_count = static_cast<uchar>(i);
  in >> i;
  chat.color_count = static_cast<Color>(i);
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

Color Chat::getColors() const noexcept {
  return color_count;
}

bool Chat::setColors(int64_t colors) noexcept {
  if (colors < MIN_COLORS || colors > MAX_COLORS) {
    return false;
  }
  color_count = static_cast<Color>(colors);
  return true;
}
