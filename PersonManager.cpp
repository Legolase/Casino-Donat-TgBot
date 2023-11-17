//
// Created by nikita on 24.08.23.
//

#include "PersonManager.h"
#include "GameEventHandler.h"
#include "Globals.h"
#include "rand.h"
#include <fstream>
#include <string>

void PersonManager::start() {
  worker = std::thread([&] { thread_main(); });
}

void PersonManager::thread_main() {
  load_user_data();
  load_chat_data();

  while (true) {
    auto opt_request = person_stream.pop();

    if (!opt_request.has_value()) {
      std::cout << "Exit person_stream\n";
      save_user_data();
      save_chat_data();
      return;
    }

    PersonRequest& request = **opt_request;
    Person* user_from = getPersonSubject(request);
    Person* user_to = getPersonObject(request);
    Chat* chat = getChat(request);

    if (request.type == PersonRequest::Type::Exit) {
      std::cout << "Close person_stream\n";
      person_stream.close();
    }
    else if (request.type == PersonRequest::Type::GameRegist) {
      game_stream.push(std::make_shared<GameRequest>(GameRequest::Type::Regist, -1, request.chat_id, request.message_id,
                                                     chat->getBit(), chat->getColors(), chat->getBots()));
    }
    else if (request.type == PersonRequest::Type::ReturnMoney) {
      user_from->close_bit();
      user_from->shift_balance(request.counter);
    }
    else if (request.type == PersonRequest::Type::GetBalance) {
      std::lock_guard lg(tgbot_mutex);
      bot.getApi().sendMessage(request.chat_id,
                               std::string("<b>") + SEARCH + "Ваш баланс:</b> " + intToCoins(user_from->get_balance()),
                               false, request.message_id, nullptr, "HTML");
    }
    else if (request.type == PersonRequest::Type::SpyBalance) {
      std::lock_guard lg(tgbot_mutex);
      bot.getApi().sendMessage(request.chat_id,
                               std::string("<b>") + SEARCH + "Баланс:</b> " + intToCoins(user_to->get_balance()), false,
                               request.message_id, nullptr, "HTML");
    }
    else if (request.type == PersonRequest::Type::Farm) {
      if (user_from->has_active_bit()) {
        bot.getApi().sendMessage(request.chat_id, std::string(WARN) + "Во время игры фармить нельзя", false,
                                 request.message_id);
      }
      else if (user_from->can_farm()) {
        user_from->update_farm();
        auto farm_amount = random(MIN_FARM_AMOUNT, MAX_FARM_AMOUNT);
        user_from->shift_balance(farm_amount);
        bot.getApi().sendMessage(request.chat_id, std::string(OK) + "На счёт зачислено: " + intToCoins(farm_amount), false,
                                 request.message_id);
      }
      else {
        std::lock_guard lg(tgbot_mutex);
        bot.getApi().sendMessage(request.chat_id,
                                 std::string(WARN) + "Используй /farm:\n  1) не чаще " + intToTime(farm_time), false,
                                 request.message_id);
      }
    }
    else if (request.type == PersonRequest::Type::AddBit) {
      if (!user_from->add_bit()) {
        std::lock_guard lg(tgbot_mutex);
        bot.getApi().sendMessage(request.chat_id,
                                 std::string(WARN) + getUserName(bot, request.chat_id, request.user_id_from) +
                                     ". Вы уже сделали максимальное кол-во ставок. Подождите)",
                                 false, 0, nullptr, "HTML");
      }
      else if (!user_from->shift_balance(-request.counter)) {
        user_from->close_bit();
        const char* no_money = " Недостаточно средств для ставки. Используйте команду /farm";
        std::lock_guard lg(tgbot_mutex);
        bot.getApi().sendMessage(request.chat_id,
                                 std::string(WARN) + getUserName(bot, request.chat_id, request.user_id_from) + +no_money,
                                 false, 0, nullptr, "HTML");
      }
      else {
        game_stream.push(std::make_shared<GameRequest>(GameRequest::Type::AddBit, request.user_id_from, request.chat_id,
                                                       request.message_id, request.counter, request.color));
      }
    }
    else if (request.type == PersonRequest::Type::Help) {
      // check valid balance
      if (!(user_from->shift_balance(-request.counter))) {
        std::lock_guard lg(tgbot_mutex);

        bot.getApi().sendMessage(request.chat_id, std::string(FAIL) + " Недостаточно средств", false, request.message_id);
        continue;
      }

      std::lock_guard lg(tgbot_mutex);
      if (user_to->shift_balance(request.counter)) {
        bot.getApi().sendMessage(request.chat_id, std::string(OK) + " Успешный перевод: " + intToCoins(request.counter),
                                 false, request.message_id);
      }
      else {
        bot.getApi().sendMessage(
            request.chat_id, "Ошибка!!! Не удалось зачислить деньги. @MetaTrigger, верните отправленную сумму отправителю",
            false, request.message_id);
      }
    }
    else if (request.type == PersonRequest::Type::SetBot) {
      std::lock_guard lg(tgbot_mutex);
      if (chat->setBots(request.counter)) {
        bot.getApi().sendMessage(request.chat_id, std::string(OK) + "Количество ботов успешно изменено " + BOT, false,
                                 request.message_id, nullptr, "HTML");
      }
      else {
        bot.getApi().sendMessage(request.chat_id,
                                 std::string(WARN) + "Допустимое кол-во ботов: 0-" + std::to_string(max_bots) + BOT, false,
                                 request.message_id, nullptr, "HTML");
      }
    }
    else if (request.type == PersonRequest::Type::SetBit) {
      std::lock_guard lg(tgbot_mutex);
      if (chat->setBit(request.counter)) {
        bot.getApi().sendMessage(request.chat_id, std::string(OK) + "Стоимость ставки в успешно изменена", false,
                                 request.message_id);
      }
      else {
        bot.getApi().sendMessage(request.chat_id, std::string(WARN) + "Минимальная ставка: " + intToCoins(MINIMUM_BIT),
                                 false, request.message_id);
      }
    }
    else if (request.type == PersonRequest::Type::GetSettings) {
      std::lock_guard lg(tgbot_mutex);
      bot.getApi().sendMessage(request.chat_id,
                               std::string("") + SEARCH + "\n<b>Установлено ботов:</b> " + std::to_string(chat->getBots()) +
                                   BOT + "\n<b>Цена ставок:</b> " + intToCoins(chat->getBit()) + "\n<b>Кол-во цветов:</b> " +
                                   std::to_string(static_cast<int>(chat->getColors())),
                               false, request.message_id, nullptr, "HTML");
    }
    else if (request.type == PersonRequest::Type::SetColor) {
      std::lock_guard lg(tgbot_mutex);
      if (chat->setColors(request.counter)) {
        bot.getApi().sendMessage(request.chat_id, std::string(OK) + " Кол-во цветов в успешно изменено", false,
                                 request.message_id);
      }
      else {
        bot.getApi().sendMessage(request.chat_id,
                                 std::string(WARN) + " Допустимое кол-во цветов: " + std::to_string(MIN_COLORS) + '-' +
                                     std::to_string(MAX_COLORS),
                                 false, request.message_id);
      }
    }
  }
}

PersonManager::PersonManager(TgBot::Bot& b) noexcept : bot(b) {}

PersonManager::~PersonManager() {
  person_stream.close();
  worker.join();
}

void PersonManager::load_user_data() {
  std::ifstream in("data.ini");
  if (!in) {
    std::cout << "Нет данных для загрузки\n";
    return;
  }
  int n;
  in >> n;

  int64_t id;
  Person p;
  for (int i = 0; i < n; ++i) {
    in >> id >> p;
    registratedUsers.emplace(id, p);
  }
}

void PersonManager::save_user_data() {
  std::ofstream out("data.ini");
  if (!out) {
    std::cout << "Данные потеряны\n";
    return;
  }
  out << registratedUsers.size() << '\n';
  for (auto const& elem : registratedUsers) {
    out << elem.first << ' ' << elem.second << '\n';
  }
}

std::map<int64_t, Person>::iterator PersonManager::registUser(int64_t id) {
  auto pr = registratedUsers.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple());
  return pr.first;
}

/*
    AddBit,       user_from   r
    Exit,         -
    Farm,         user_from   r
    GameRegist,   -
    GetBalance,   user_from   n
    Help,         user_from   r
    ReturnMoney,  user_from   r
    SpyBalance,   -
    SetBot,       -
    SetBit,       -
    GetSettings,  -
    SetColor      -
*/
Person* PersonManager::getPersonSubject(PersonRequest const& request) {
  decltype(registratedUsers)::iterator user_it;

  if (request.type == PersonRequest::Type::GameRegist || request.type == PersonRequest::Type::SpyBalance ||
      request.type == PersonRequest::Type::SetBot || request.type == PersonRequest::Type::SetBit ||
      request.type == PersonRequest::Type::GetSettings || request.type == PersonRequest::Type::Exit ||
      request.type == PersonRequest::Type::SetColor)
  {
    return &default_person;
  }

  user_it = registratedUsers.find(request.user_id_from);
  if (request.type == PersonRequest::Type::GetBalance) {
    if (user_it == registratedUsers.end()) {
      return &default_person;
    }
    else {
      return &(user_it->second);
    }
  }

  if (user_it == registratedUsers.end()) {
    user_it = registUser(request.user_id_from);
  }
  return &(user_it->second);
}
/*
    AddBit,       -
    Exit,         -
    Farm,         -
    GameRegist,   -
    GetBalance,   -
    Help,         user_to   r
    ReturnMoney,  -
    SpyBalance,   user_to   n
    SetBot,       -
    SetBit,       -
    GetSettings,  -
    SetColor      -
*/
Person* PersonManager::getPersonObject(const PersonRequest& request) {
  decltype(registratedUsers)::iterator user_it;

  if (request.type != PersonRequest::Type::Help && request.type != PersonRequest::Type::SpyBalance) {
    return &default_person;
  }
  user_it = registratedUsers.find(request.user_id_to);
  if (request.type == PersonRequest::Type::Help) {
    if (user_it == registratedUsers.end()) {
      user_it = registUser(request.user_id_to);
    }
    return &(user_it->second);
  }

  if (user_it == registratedUsers.end()) {
    return &default_person;
  }
  return &(user_it->second);
}
/*
  AddBit,       -
  Exit,         -
  Farm,         -
  GameRegist,   chat r
  GetBalance,   -
  Help,         -
  ReturnMoney,  chat r
  SpyBalance,   -
  SetBot        chat r
  SetBit,       chat r
  GetSettings,  chat r
  SetColor      chat r
*/
Chat* PersonManager::getChat(const PersonRequest& request) {
  decltype(registratedChats)::iterator chat_it;

  if (request.type != PersonRequest::Type::GameRegist && request.type != PersonRequest::Type::ReturnMoney &&
      request.type != PersonRequest::Type::SetBot && request.type != PersonRequest::Type::SetBit &&
      request.type != PersonRequest::Type::GetSettings && request.type != PersonRequest::Type::SetColor)
  {
    return nullptr;
  }

  chat_it = registratedChats.find(request.chat_id);
  if (chat_it == registratedChats.end()) {
    chat_it = registChat(request.chat_id);
  }
  return &(chat_it->second);
}

std::map<int64_t, Chat>::iterator PersonManager::registChat(int64_t id) {
  auto pr = registratedChats.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple());
  return pr.first;
}

void PersonManager::load_chat_data() {
  std::ifstream in("chats.ini");
  if (!in) {
    std::cout << "Нет данных для загрузки\n";
    return;
  }
  int n;
  in >> n;

  int64_t id;
  Chat ch;
  for (int i = 0; i < n; ++i) {
    in >> id >> ch;
    registratedChats.emplace(id, ch);
  }
}

void PersonManager::save_chat_data() {
  std::ofstream out("chats.ini");
  if (!out) {
    std::cout << "Данные потеряны\n";
    return;
  }
  out << registratedChats.size() << '\n';
  for (auto const& elem : registratedChats) {
    out << elem.first << ' ' << elem.second << '\n';
  }
}
