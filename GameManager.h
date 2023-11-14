//
// Created by nikita on 24.08.23.
//

#ifndef CASINO_GAMEMANAGER_H
#define CASINO_GAMEMANAGER_H

#include "Chat.h"
#include "Game.h"
#include "GameEventHandler.h"
#include <thread>

class GameManager {
public:
  explicit GameManager(TgBot::Bot& b) noexcept;
  ~GameManager();
  void start();
  void thread_main();

  // returns time for sleep
  int checkGames();

  // returns message_id
  int32_t sendMessage(int64_t group_id, int64_t bt);
private:
  void load_data();
  void save_data();

  std::map<int64_t, Game> current_games;
  std::thread worker;
  TgBot::Bot& bot;
};

static_assert(sizeof(GameManager) == 0);
static_assert(sizeof(std::map<int64_t, std::pair<>>) == 0);

#endif // CASINO_GAMEMANAGER_H
