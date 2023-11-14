//
// Created by nikita on 24.08.23.
//

#ifndef CASINO_GAME_H
#define CASINO_GAME_H

#include "Color.h"
#include "GameEventHandler.h"
#include "Globals.h"
#include "Time.h"
#include <map>
#include <set>
#include <tgbot/tgbot.h>
#include <vector>

struct Game {
  static constexpr int MAX_LIFETIME = 35;

  explicit Game(int32_t ms_id, int64_t bt, int bots_in);
  Game(Game const&) = delete;

  bool update(TgBot::Bot& bot, int64_t group_id);

  void addBit(TgBot::Bot& bot, GameRequest const& req);

  void addBotBit(TgBot::Bot& bot, Color col);

  [[nodiscard]] int lifetime() const noexcept;
private:

  std::vector<std::set<int64_t>> goals{static_cast<int>(Color::last)};
  std::vector<std::pair<int64_t, int32_t>> bit_message;
  int64_t const own_bit;
  int32_t const message_id;
  int bit_counter{0};
  int start_time{time()};
  int const bot_in;
};

#endif // CASINO_GAME_H
