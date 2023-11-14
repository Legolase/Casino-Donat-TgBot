//
// Created by nikita on 24.08.23.
//

#ifndef CASINO_PERSONMANAGER_H
#define CASINO_PERSONMANAGER_H

#include "Person.h"
#include "PersonEventHandler.h"
#include <map>
#include <tgbot/tgbot.h>
#include <thread>

class PersonManager {

public:
  explicit PersonManager(TgBot::Bot& b) noexcept;
  ~PersonManager();

  void start();
  void thread_main();

  std::map<int64_t, Person>::iterator regist(int64_t id, std::map<int64_t, Person>::iterator);

private:
  void load_data();
  void save_data();

  std::map<int64_t, Person> registrated_users;
  TgBot::Bot& bot;
  std::thread worker;
};

#endif // CASINO_PERSONMANAGER_H
