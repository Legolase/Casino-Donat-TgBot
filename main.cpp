#include "GameManager.h"
#include "Globals.h"
#include "PersonManager.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <tgbot/tgbot.h>

#define NO_PROCESS 0

TgBot::Bot bot("6653170160:AAHoOvyhZw10GihbNrjKfvq7LXPwGXPEzqU");
//TgBot::Bot bot("6798304137:AAFSxIMdZSIeQahfgxdVahPU2pleL7aj1pc"); //testbot
PersonManager person_manager(bot);
GameManager game_manager(bot);

void end(int s) {
  printf("END\n");
  person_stream.push(std::make_shared<PersonRequest>(PersonRequest::Type::Exit, 1));
  game_stream.push(std::make_shared<GameRequest>(GameRequest::Type::Exit, 1));
  exit(0);
}

void log(TgBot::Message::Ptr const& message) {
  auto now = time(original_t{});
  char timeString[std::size("yyyy-mm-ddThh:mm:ssZ")];
  std::strftime(std::data(timeString), std::size(timeString), "%T", std::gmtime(&now));
  if (message->chat->type != TgBot::Chat::Type::Private) {
    printf("%s|-|%s/%s/%s/%li : %s\n", &timeString, message->chat->title.c_str(), message->from->firstName.c_str(),
           message->from->username.c_str(), message->from->id, message->text.c_str());
  }
  else {
    printf("%s|-|PRIVATE/%s/%s/%li : %s\n", &timeString, message->from->firstName.c_str(), message->from->username.c_str(),
           message->from->id, message->text.c_str());
  }
}

void log(TgBot::CallbackQuery::Ptr const& query) {
  auto now = time(original_t{});
  char timeString[std::size("yyyy-mm-ddThh:mm:ssZ")];
  std::strftime(std::data(timeString), std::size(timeString), "%T", std::gmtime(&now));
  if (query->message->chat->type != TgBot::Chat::Type::Private) {
    printf("%s|-|%s/%s/%s/%li : %s\n", &timeString, query->message->chat->title.c_str(), query->from->firstName.c_str(),
           query->from->username.c_str(), query->from->id, query->data.c_str());
  }
  else {
    printf("%s|-|PRIVATE/%s/%s/%li : %s\n", &timeString, query->from->firstName.c_str(), query->from->username.c_str(),
           query->from->id, query->data.c_str());
  }
}

void getBalance(TgBot::Message::Ptr const& message) {
  if (message->replyToMessage != nullptr && !(message->replyToMessage->from->isBot)) {
    person_stream.push(std::make_shared<PersonRequest>(PersonRequest::Type::SpyBalance, message->from->id, message->chat->id,
                                                       message->replyToMessage->messageId, -1,
                                                       message->replyToMessage->from->id));
  }
  else {
    person_stream.push(std::make_shared<PersonRequest>(PersonRequest::Type::GetBalance, message->from->id, message->chat->id,
                                                       message->messageId));
  }
}

void addEventHandlers() {
  bot.getEvents().onCommand("start", [&](TgBot::Message::Ptr message) {
    if (NO_PROCESS) return;
    std::lock_guard lg(tgbot_mutex);
    bot.getApi().sendMessage(
        message->chat->id,
        "<b>Привет, " + message->from->firstName +
            "!</b> Я рад приветствовать тебя в мире развлечений и азарта! Я здесь, чтобы сделать твоё время с нами "
            "незабываемым и захватывающим. Важно отметить, что для моего использования не требуются реальные "
            "деньги!\n\nИгровая валюта " +
            COIN +
            "\n\nИнструкция по установке: /setup"
            "\nПосмотреть список команд: /list",
        false, 0, nullptr, "HTML");
    log(message);
  });
  bot.getEvents().onCommand("list", [&](TgBot::Message::Ptr message) {
    if (NO_PROCESS) return;
    std::lock_guard lg(tgbot_mutex);
    bot.getApi().sendMessage(message->chat->id,
                             std::string("<b>Вот некоторые из моих возможностей:</b>"
                                         "\n  1) /newgame - создать игру"
                                         "\n  2) /balance - узнать свой баланс"
                                         "\n  3) /settings - настройки игры"
                                         "\n  4) /farm - фарм ") +
                                 COIN +
                                 "\n  5) <code>.bit</code> - задать ставку"
                                 "\n  6) <code>.bot</code> - задать ботов"
                                 "\n  7) <code>.color</code> - задать кол-во цветов"
                                 "\n  8) <code>.help</code> - перевести деньги игроку",
                             false, 0, nullptr, "HTML");
    log(message);
  });
  bot.getEvents().onCommand("setup", [&](TgBot::Message::Ptr message) {
    if (NO_PROCESS) return;
    std::lock_guard lg(tgbot_mutex);
    bot.getApi().sendMessage(message->chat->id,
                             "<b>Инструкция по установке</b>"
                             "\n  1) Нажмите на <b>название бота</b>;"
                             "\n  2) Нажмите на кнопку <b>Добавить в группу</b>;"
                             "\n  3) Выберите Вашу <b>группу</b>;"
                             "\n  4) Нажмите на галочку в верхнем правом углу, для корректной работы бота;",
                             false, 0, nullptr, "HTML");
    log(message);
  });
  bot.getEvents().onCommand("balance", [&](TgBot::Message::Ptr message) {
    if (NO_PROCESS) return;
    getBalance(message);
    log(message);
  });
  bot.getEvents().onCommand("farm", [&](TgBot::Message::Ptr message) {
    if (NO_PROCESS) return;
    person_stream.push(std::make_shared<PersonRequest>(PersonRequest::Type::Farm, message->from->id, message->chat->id,
                                                       message->messageId));
    log(message);
  });
  bot.getEvents().onCommand("newgame", [&](TgBot::Message::Ptr message) {
    if (NO_PROCESS) return;
    if (message->chat->type == TgBot::Chat::Type::Private) {
      std::lock_guard lk(tgbot_mutex);
      bot.getApi().sendMessage(message->chat->id,
                               std::string(SEARCH) + "Рекомендуется к использованию в групповых чатах. /setup");
    }
    person_stream.push(std::make_shared<PersonRequest>(PersonRequest::Type::GameRegist, message->from->id, message->chat->id,
                                                       message->messageId));
    log(message);
  });
  bot.getEvents().onCommand("getcode", [&](TgBot::Message::Ptr message) {
    if (NO_PROCESS) return;
    std::stringstream stream;
    std::string word;
    stream << message->text;
    stream.ignore(100, ' ');
    stream >> word;
    for (auto const& elem : word) {
      printf("%X\n", static_cast<int>(elem));
    }
    log(message);
  });
  bot.getEvents().onCommand("settings", [&](TgBot::Message::Ptr message) {
    if (NO_PROCESS) return;
    person_stream.push(std::make_shared<PersonRequest>(PersonRequest::Type::GetSettings, message->from->id, message->chat->id,
                                                       message->messageId));
    log(message);
  });
  bot.getEvents().onCallbackQuery([&](TgBot::CallbackQuery::Ptr const query) {
    if (NO_PROCESS) return;
    std::stringstream stream;
    stream << std::string(query->data);
    int color;
    int64_t bt;
    stream >> color >> bt;
    person_stream.push(std::make_shared<PersonRequest>(PersonRequest::Type::AddBit, query->from->id,
                                                       query->message->chat->id, query->message->messageId, bt, -1,
                                                       static_cast<Color>(color)));
    std::lock_guard lk(tgbot_mutex);
    try {
      bot.getApi().answerCallbackQuery(query->id);
    } catch (...) {
    }
    log(query);
  });
  bot.getEvents().onAnyMessage([&](TgBot::Message::Ptr message) {
    if (NO_PROCESS) return;
    if (!StringTools::startsWith(message->text, ".")) {
      return;
    }
    else if (StringTools::startsWith(message->text, ".help")) {
      if (message->text == ".help") {
        std::lock_guard lg(tgbot_mutex);
        bot.getApi().sendMessage(message->chat->id,
                                 std::string(WARN) + "Вы не указали сумму после команды.\nПример \"<code>.help 15</code>\"",
                                 false, message->messageId, nullptr, "HTML");
        return;
      }
      else if (message->replyToMessage == nullptr) {
        std::lock_guard lg(tgbot_mutex);
        bot.getApi().sendMessage(message->chat->id,
                                 std::string(WARN) + "Вы не указали получателя суммы. Чтобы указать, "
                                                     "сошлитесь на сообщение получателя.",
                                 false, message->messageId);
        return;
      }
      else if (message->replyToMessage->from->id == message->from->id) {
        std::lock_guard lg(tgbot_mutex);
        bot.getApi().sendMessage(message->chat->id, std::string(HAHA) + "Хорошая шутка. Ваш баланс не изменён", false,
                                 message->messageId);
        return;
      }
      else if (message->replyToMessage->from->isBot) {
        std::lock_guard lg(tgbot_mutex);
        bot.getApi().sendMessage(message->chat->id, std::string(HAHA) + "Спасибо, конечно, но я и так богат", false,
                                 message->messageId);
        return;
      }
      std::stringstream stream;
      std::string command;
      int64_t amount;
      stream << message->text;
      stream >> command;

      if (!(stream >> amount) || amount <= 0) {
        std::lock_guard lg(tgbot_mutex);
        bot.getApi().sendMessage(message->chat->id, std::string(WARN) + "Неверно введена сумма для перевода", false,
                                 message->messageId);
        return;
      }

      person_stream.push(std::make_shared<PersonRequest>(PersonRequest::Type::Help, message->from->id, message->chat->id,
                                                         message->messageId, amount, message->replyToMessage->from->id));
      log(message);
    }
    else if (StringTools::startsWith(message->text, ".bit")) {
      std::stringstream stream;
      int64_t val;
      stream << message->text;
      stream.ignore(100, ' ');
      if (stream >> val) {
        person_stream.push(std::make_shared<PersonRequest>(PersonRequest::Type::SetBit, message->from->id, message->chat->id,
                                                           message->messageId, val));
      }
      else {
        std::lock_guard lg(tgbot_mutex);
        bot.getApi().sendMessage(message->chat->id,
                                 std::string(WARN) +
                                     "Вы не указали стоимость ставки.\nПример: \"<code>.bit 15</code>\"",
                                 false, message->messageId, nullptr, "HTML");
      }
      log(message);
    }
    else if (StringTools::startsWith(message->text, ".bot")) {
      std::stringstream stream;
      int64_t val;
      stream << message->text;
      stream.ignore(100, ' ');
      if (stream >> val) {
        person_stream.push(std::make_shared<PersonRequest>(PersonRequest::Type::SetBot, message->from->id,
                                                           message->chat->id, message->messageId, val));
      }
      else {
        std::lock_guard lg(tgbot_mutex);
        bot.getApi().sendMessage(message->chat->id,
                                 std::string(WARN) +
                                     "Вы не указали кол-во ботов.\nПример: \"<code>.bot 3</code>\"",
                                 false, message->messageId, nullptr, "HTML");
      }
      log(message);
    }
    else if (StringTools::startsWith(message->text, ".color")) {
      std::stringstream stream;
      int64_t val;
      stream << message->text;
      stream.ignore(100, ' ');
      if (stream >> val) {
        person_stream.push(std::make_shared<PersonRequest>(PersonRequest::Type::SetColor, message->from->id,
                                                           message->chat->id, message->messageId, val));
      }
      else {
        std::lock_guard lg(tgbot_mutex);
        bot.getApi().sendMessage(message->chat->id,
                                 std::string(WARN) +
                                     "Вы не указали кол-во ботов.\nПример: \"<code>.color 4</code>\"",
                                 false, message->messageId, nullptr, "HTML");
      }
      log(message);
    }
  });
}

int main() {
  bot.getApi().sendMessage(406004592, "restart");

  addEventHandlers();

  person_manager.start();
  game_manager.start();
  signal(SIGINT, end);

  try {
    printf("Restart\n");
    TgBot::TgLongPoll longPoll(bot);
    while (true) {
      longPoll.start();
    }
  } catch (TgBot::TgException& e) {
    printf("error tg: %s\n", e.what());
  } catch (std::exception& e) {
    printf("error std: %s\n", e.what());
  }
  end(0);
}