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
      if (request.type == PersonRequest::Type::GetBalance || request.type == PersonRequest::Type::SpyBalance ||
          request.type == PersonRequest::Type::GetBots || request.type == PersonRequest::Type::GameRegist)
      {
        bare_person = &default_user;
      }
      else {
        user_it = regist(request.user_id_from, user_it);
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
    else if (request.type == PersonRequest::Type::GameRegist) {
      game_stream.push(std::make_shared<GameRequest>(GameRequest::Type::Regist, request.user_id_from, request.group_id,
                                                     request.message_id, user_from.getBit(), Color::last,
                                                     user_from.getBots()));
    }
    else if (request.type == PersonRequest::Type::ReturnMoney) {
      user_from.in_game.erase(request.group_id);
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
      if (user_from.in_game.size() > 0) {
        bot.getApi().sendMessage(request.group_id, std::string(WARN) + "Во время игры фармить нельзя", false,
                                 request.message_id);
      }
      else if (user_from.can_farm()) {
        user_from.update_farm();
        user_from.shift_balance(Person::FARM_AMOUNT);
        bot.getApi().sendMessage(request.group_id, std::string(OK) + "На счёт зачислено: " + intToCoins(Person::FARM_AMOUNT),
                                 false, request.message_id);
      }
      else {
        std::lock_guard lg(tgbot_mutex);
        bot.getApi().sendMessage(request.group_id,
                                 std::string(WARN) + "Используй /farm:\n  1) не чаще " + intToTime(farm_time),
                                 false, request.message_id);
      }
    }
    else if (request.type == PersonRequest::Type::AddBit) {
      if (user_from.in_game.contains(request.group_id)) {
        std::lock_guard lg(tgbot_mutex);
        try {
          auto u = bot.getApi().getChatMember(request.group_id, request.user_id_from)->user;
          if (u->username.size() > 0) {
            bot.getApi().sendMessage(request.group_id,
                                     std::string(WARN) + '@' + u->username + ". Вы уже сделали ставку. Подождите)");
          }
          else {
//            TgBot::MessageEntity::Ptr entity(new TgBot::MessageEntity);
//            entity->type = TgBot::MessageEntity::Type::TextMention;
//            entity->user = u;
//            entity->offset = 4;
//            entity->length = 2;
            bot.getApi().sendMessage(request.group_id, std::string(WARN) + "Вы уже сделали ставку. Подождите)", false, 0,
                                     nullptr, "", false/*, {entity}*/);
          }
        } catch (TgBot::TgException& e) {
          std::cout << e.what() << '\n';
          bot.getApi().sendMessage(request.group_id, std::string(WARN) + "<i>Unknown</i>. Вы уже сделали ставку. Подождите)",
                                   false, 0, nullptr, "HTML");
        }
      }
      else if (!user_from.shift_balance(-request.counter)) {
        const char* no_money = "Недостаточно средств для ставки. Используйте команду /farm";
        std::lock_guard lg(tgbot_mutex);
        try {
          auto u = bot.getApi().getChatMember(request.group_id, request.user_id_from)->user;
          if (u->username.size() > 0) {
            bot.getApi().sendMessage(request.group_id, std::string(WARN) + " @" + u->username + ' ' + no_money);
          }
          else {
            TgBot::MessageEntity::Ptr entity(new TgBot::MessageEntity);
            entity->type = TgBot::MessageEntity::Type::TextMention;
            entity->user = u;
            entity->offset = 10;
            entity->length = 3;
            bot.getApi().sendMessage(request.group_id, std::string(WARN) + no_money, false, 0, nullptr, "", false, {entity});
          }
        } catch (...) {
          bot.getApi().sendMessage(request.group_id, std::string(WARN) + no_money, false, 0, nullptr, "HTML");
        }
      }
      else {
        user_from.in_game.insert(request.group_id);
        game_stream.push(std::make_shared<GameRequest>(GameRequest::Type::AddBit, request.user_id_from, request.group_id,
                                                       request.message_id, request.counter, request.color));
      }
    }
    else if (request.type == PersonRequest::Type::SetBit) {
      std::lock_guard lg(tgbot_mutex);
      if (user_from.setBit(request.counter)) {
        bot.getApi().sendMessage(request.group_id, std::string(OK) + "Стоимость ставки в Ваших играх успешно изменена",
                                 false, request.message_id);
      }
      else {
        bot.getApi().sendMessage(request.group_id,
                                 std::string(WARN) + "Минимальная ставка: " + intToCoins(Person::MINIMUM_BIT), false,
                                 request.message_id);
      }
    }
    else if (request.type == PersonRequest::Type::Help) {
      // check valid balance
      if (!(user_from.shift_balance(-request.counter))) {
        std::lock_guard lg(tgbot_mutex);

        bot.getApi().sendMessage(request.group_id, std::string(FAIL) + "Недостаточно средств", false, request.message_id);
        continue;
      }

      std::lock_guard lg(tgbot_mutex);
      if (user_to.shift_balance(request.counter)) {
        bot.getApi().sendMessage(request.group_id, std::string(OK) + "Успешный перевод: " + intToCoins(request.counter),
                                 false, request.message_id);
      }
      else {
        bot.getApi().sendMessage(
            request.group_id, "Ошибка!!! Не удалось зачислить деньги. @MetaTrigger, верните отправленную сумму отправителю",
            false, request.message_id);
      }
    }
    else if (request.type == PersonRequest::Type::GetBots) {
      std::lock_guard lg(tgbot_mutex);
      bot.getApi().sendMessage(request.group_id,
                               std::string("<b>") + SEARCH + "Установлено ботов:</b> " +
                                   std::to_string(user_from.getBots()) + BOT,
                               false, request.message_id, nullptr, "HTML");
    }
    else if (request.type == PersonRequest::Type::SetBots) {
      std::lock_guard lg(tgbot_mutex);
      if (user_from.setBots(request.counter)) {
        bot.getApi().sendMessage(request.group_id, std::string(OK) + "Количество ботов успешно изменено " + BOT, false,
                                 request.message_id, nullptr, "HTML");
      }
      else {
        bot.getApi().sendMessage(request.group_id,
                                 std::string(WARN) + "Допустимое кол-во ботов: 0-" + std::to_string(max_bots) + BOT, false,
                                 request.message_id, nullptr, "HTML");
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
  int64_t id, balance, bt;
  int last_farm, bot_num;
  in >> n;

  std::map<int64_t, Person>::iterator it;
  for (int i = 0; i < n; ++i) {
    in >> id >> balance >> last_farm >> bt >> bot_num;
    registrated_users.insert(std::make_pair(id, Person(balance, last_farm, bt, bot_num)));
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
    out << elem.first << ' ' << elem.second.get_balance() << ' ' << elem.second.get_last_farm() << ' '
        << elem.second.getBit() << ' ' << elem.second.getBots() << '\n';
  }
}

std::map<int64_t, Person>::iterator PersonManager::regist(int64_t id, std::map<int64_t, Person>::iterator user) {
  if (user != registrated_users.end()) {
    std::cout << "Warning: вызван regist для зареганного юзера";
    return user;
  }
  else {
    auto pr = registrated_users.insert(std::make_pair(id, Person{}));
    return pr.first;
  }
}
