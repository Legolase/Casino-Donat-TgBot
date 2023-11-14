//
// Created by nikita on 24.08.23.
//

#ifndef CASINO_GAMEMANAGER_H
#define CASINO_GAMEMANAGER_H

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
  TgBot::Bot& bot;
  std::thread worker;
  std::map<int64_t, Game> current_games;
};

#endif // CASINO_GAMEMANAGER_H
