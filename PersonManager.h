//
// Created by nikita on 24.08.23.
//

#ifndef CASINO_PERSONMANAGER_H
#define CASINO_PERSONMANAGER_H

#include "Chat.h"
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

  std::map<int64_t, Person>::iterator registUser(int64_t id);
  std::map<int64_t, Chat>::iterator registChat(int64_t id);

private:
  void load_data();
  void save_data();

  Person* getPersonSubject(PersonRequest const& request);
  Person* getPersonObject(PersonRequest const& request);
  Chat*   getChat(PersonRequest const& request);

  std::map<int64_t, Person> registratedUsers;
  std::map<int64_t, Chat> registratedChats;
  TgBot::Bot& bot;
  std::thread worker;

  Person default_person;
};

#endif // CASINO_PERSONMANAGER_H
