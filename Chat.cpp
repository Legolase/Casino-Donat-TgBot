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
