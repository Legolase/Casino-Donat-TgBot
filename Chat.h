//
// Created by nikita on 14.11.23.
//

#ifndef CASINO_CHAT_H
#define CASINO_CHAT_H

#include "Globals.h"
#include "Color.h"
#include <cstdint>
#include <istream>
#include <ostream>

struct Chat {
  Chat() noexcept = default;

  friend std::ostream& operator<<(std::ostream& out, Chat const& chat);
  friend std::istream& operator>>(std::istream& in, Chat& chat);

  int64_t getBit() const noexcept;
  uchar getBots() const noexcept;
  Color getColors() const noexcept;

  bool setBots(int bots) noexcept;
  bool setBit(int64_t val) noexcept;
  bool setColors(int64_t colors) noexcept;
private:
  int64_t bit{MINIMUM_BIT};
  uchar bot_count{DEFAULT_BOTS};
  Color color_count{DEFAULT_COLORS};
};

#endif // CASINO_CHAT_H
