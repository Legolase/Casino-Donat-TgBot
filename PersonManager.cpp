//
// Created by nikita on 24.08.23.
//

#include "PersonManager.h"
#include "GameEventHandler.h"
#include "Globals.h"
#include <fstream>
#include <string>

void PersonManager::start() {
  worker = std::thread([&] { thread_main(); });
}

void PersonManager::thread_main() {
  load_data();

  auto default_user = Person{};
  while (true) {
    auto opt_request = person_stream.pop();

    if (!opt_request.has_value()) {
      std::cout << "Exit person_stream\n";
      save_data();
      return;
    }

    PersonRequest& request = **opt_request;
    Person* bare_person;

    auto user_it = registrated_users.find(request.user_id_from);
    if (user_it == registrated_users.end()) {
      if (request.type == PersonRequest::Type::GetBalance || request.type == PersonRequest::Type::SpyBalance)
      {
        bare_person = &default_user;
      }
      else {
        user_it = regist(request.user_id_from);
        bare_person = &(user_it->second);
      }
    }
    else {
      bare_person = &(user_it->second);
    }
    Person& user_from = *bare_person;

    user_it = registrated_users.find(request.user_id_to);
    if (user_it == registrated_users.end()) {
      bare_person = &default_user;
    }
    else {
      bare_person = &(user_it->second);
    }
    Person& user_to = *bare_person;

    if (request.type == PersonRequest::Type::Exit) {
      std::cout << "Close person_stream\n";
      person_stream.close();
    }
    else if (request.type == PersonRequest::Type::ReturnMoney) {
      user_from.close_bit();
      user_from.shift_balance(request.counter);
    }
    else if (request.type == PersonRequest::Type::GetBalance) {
      std::lock_guard lg(tgbot_mutex);
      bot.getApi().sendMessage(request.group_id,
                               std::string("<b>") + SEARCH + "Ваш баланс:</b> " + intToCoins(user_from.get_balance()), false,
                               request.message_id, nullptr, "HTML");
    }
    else if (request.type == PersonRequest::Type::SpyBalance) {
      std::lock_guard lg(tgbot_mutex);
      bot.getApi().sendMessage(request.group_id,
                               std::string("<b>") + SEARCH + "Баланс:</b> " + intToCoins(user_to.get_balance()), false,
                               request.message_id, nullptr, "HTML");
    }
    else if (request.type == PersonRequest::Type::Farm) {
      if (user_from.has_active_bit()) {
        bot.getApi().sendMessage(request.group_id, std::string(WARN) + "Во время игры фармить нельзя", false,
                                 request.message_id);
      }
      else if (user_from.can_farm()) {
        user_from.update_farm();
        user_from.shift_balance(FARM_AMOUNT);
        bot.getApi().sendMessage(request.group_id, std::string(OK) + "На счёт зачислено: " + intToCoins(FARM_AMOUNT), false,
                                 request.message_id);
      }
      else {
        std::lock_guard lg(tgbot_mutex);
        bot.getApi().sendMessage(request.group_id,
                                 std::string(WARN) + "Используй /farm:\n  1) не чаще " + intToTime(farm_time), false,
                                 request.message_id);
      }
    }
    else if (request.type == PersonRequest::Type::AddBit) {
      if (!user_from.add_bit()) {
        std::lock_guard lg(tgbot_mutex);
        bot.getApi().sendMessage(request.group_id,
                                 std::string(WARN) + getUserName(bot, request.group_id, request.user_id_from) +
                                     ". Вы уже сделали ставку. Подождите)",
                                 false, 0, nullptr, "HTML");
      }
      else if (!user_from.shift_balance(-request.counter)) {
        user_from.close_bit();
        const char* no_money = " Недостаточно средств для ставки. Используйте команду /farm";
        std::lock_guard lg(tgbot_mutex);
        bot.getApi().sendMessage(request.group_id,
                                 std::string(WARN) + getUserName(bot, request.group_id, request.user_id_from) +
                                     + no_money,
                                 false, 0, nullptr, "HTML");
      }
      else {
        game_stream.push(std::make_shared<GameRequest>(GameRequest::Type::AddBit, request.user_id_from, request.group_id,
                                                       request.message_id, request.counter, request.color));
      }
    }
    else if (request.type == PersonRequest::Type::Help) {
      // check valid balance
      if (!(user_from.shift_balance(-request.counter))) {
        std::lock_guard lg(tgbot_mutex);

        bot.getApi().sendMessage(request.group_id, std::string(FAIL) + " Недостаточно средств", false, request.message_id);
        continue;
      }

      std::lock_guard lg(tgbot_mutex);
      if (user_to.shift_balance(request.counter)) {
        bot.getApi().sendMessage(request.group_id, std::string(OK) + " Успешный перевод: " + intToCoins(request.counter),
                                 false, request.message_id);
      }
      else {
        bot.getApi().sendMessage(
            request.group_id, "Ошибка!!! Не удалось зачислить деньги. @MetaTrigger, верните отправленную сумму отправителю",
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

void PersonManager::load_data() {
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
    registrated_users.emplace(id, p);
  }
}

void PersonManager::save_data() {
  std::ofstream out("data.ini");
  if (!out) {
    std::cout << "Данные потеряны\n";
    return;
  }
  out << registrated_users.size() << '\n';
  for (auto const& elem : registrated_users) {
    out << elem.first << ' ' << elem.second;
  }
}

std::map<int64_t, Person>::iterator PersonManager::regist(int64_t id) {
  auto pr = registrated_users.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple());
  return pr.first;
}
