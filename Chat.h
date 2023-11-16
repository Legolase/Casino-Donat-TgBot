//
// Created by nikita on 14.11.23.
//

#ifndef CASINO_CHAT_H
#define CASINO_CHAT_H

#include "Globals.h"
#include <cstdint>
#include <istream>
#include <ostream>

struct Chat {
  Chat() noexcept = default;

  friend std::ostream& operator<<(std::ostream& out, Chat const& chat);
  friend std::istream& operator>>(std::istream& in, Chat& chat);

private:
  int64_t bit{MINIMUM_BIT};
  uchar bot_count{DEFAULT_BOTS};
};

#endif // CASINO_CHAT_H
