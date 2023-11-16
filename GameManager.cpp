//
// Created by nikita on 24.08.23.
//

#include "GameManager.h"
#include "Globals.h"
#include "PersonEventHandler.h"
#include <utility>
#include <vector>

GameManager::GameManager(TgBot::Bot& b) noexcept : bot(b) {}
void GameManager::start() {
  worker = std::thread([&] { thread_main(); });
}
void GameManager::thread_main() {
  int sleep_;
  load_data();

  while (true) {
    sleep_ = checkGames();

    std::optional<std::shared_ptr<GameRequest>> opt_request;

    if (current_games.empty())
      opt_request = game_stream.pop();
    else
      opt_request = game_stream.try_pop(sleep_);

    if (game_stream.is_closed() && game_stream.empty() && current_games.empty()) {
      std::cout << "Exit game_stream\n";
      save_data();
      return;
    }

    if (!opt_request.has_value()) {
      continue;
    }

    GameRequest& request = **opt_request;

    Chat& chat = *getChatByRequest(request);

    auto game = current_games.find(request.group_id);
    Chat* bare_chat;

    if (game == current_games.end()) {
      auto chat_it = chat_settings.find(request.group_id);
      if (chat_it == chat_settings.end()) {
        bare_chat = &default_chat;
      }
    }

    if (request.type == GameRequest::Type::Exit) {
      std::cout << "Close game_stream\n";
      game_stream.close();
    }
    else if (request.type == GameRequest::Type::Regist) {
      if (game == current_games.end()) {
        current_games.emplace(
            std::piecewise_construct, std::forward_as_tuple(request.group_id),
            std::forward_as_tuple(sendMessage(request.group_id, request.own_bit), request.own_bit, request.bts_));
        try {
          std::lock_guard lg(tgbot_mutex);
          bot.getApi().deleteMessage(request.group_id, request.message_id);
        } catch (...) {
        }
      }
      else {
        std::lock_guard lg(tgbot_mutex);
        try {
          bot.getApi().deleteMessage(request.group_id, request.message_id);
        } catch (...) {
          bot.getApi().sendMessage(request.group_id, std::string(WARN) + "Нельзя запускать в группе больше одной игры");
        }
      }
    }
    else if (request.type == GameRequest::Type::AddBit) {
      if (game == current_games.end()) {
        person_stream.push(std::make_shared<PersonRequest>(PersonRequest::Type::ReturnMoney, request.user_id,
                                                           request.group_id, -1, request.own_bit));
      }
      else {
        game->second.addBit(bot, request);
      }
    }
  }
}

int GameManager::checkGames() {
  int min = Game::MAX_LIFETIME;
  int tmp;
  for (auto it = current_games.begin(); it != current_games.end();) {
    if (it->second.update(bot, it->first)) {
      it = current_games.erase(it);
    }
    else {
      tmp = Game::MAX_LIFETIME - it->second.lifetime();
      min = (min > tmp) ? tmp : min;
      ++it;
    }
  }
  return min;
}

int32_t GameManager::sendMessage(int64_t group_id, int64_t bt) {
  TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);
  std::vector<std::vector<TgBot::InlineKeyboardButton::Ptr>> buttons;
  buttons.emplace_back();
  for (int i = 0; i < static_cast<int>(Color::last); ++i) {
    buttons.back().emplace_back(new TgBot::InlineKeyboardButton);
    buttons.back().back()->text = color_text[i];
    buttons.back().back()->callbackData = std::to_string(i) + " " + std::to_string(bt);
  }
  keyboard->inlineKeyboard = std::move(buttons);
  std::lock_guard lg(tgbot_mutex);
  int32_t messageId =
      bot.getApi()
          .sendMessage(group_id,
                       std::string("<b><i>Игра начинается.</i></b>\n\nВыбери выигрышный цвет.\nЦена ставки: <b>" +
                                   intToCoins(bt) + "</b>.\nВремя принятия ставок: ") +
                           intToTime(Game::MAX_LIFETIME),
                       false, 0, keyboard, "HTML", true)
          ->messageId;
  //  bot.getApi().pinChatMessage(group_id, messageId);
  return messageId;
}

GameManager::~GameManager() {
  game_stream.close();
  worker.join();
}

void GameManager::save_data() {
  std::ofstream out("chats.ini");
  if (!out) {
    std::cout << "Данные потеряны\n";
    return;
  }
  out << chat_settings.size() << '\n';
  for (auto const& elem : chat_settings) {
    out << elem.first << ' ' << elem.second;
  }
}

void GameManager::load_data() {
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
    chat_settings.emplace(id, ch);
  }
}

Chat* GameManager::getChatByRequest(const GameRequest& request) {
  decltype(chat_settings)::iterator chat_it;
  if (request.type != GameRequest::Type::AddBit) {
    chat_it = chat_settings.find(request.group_id);
    if (request.type == GameRequest::Type::SetBots || request.type == GameRequest::Type::SetBitValue) {
      if (chat_it == chat_settings.end()) {
        chat_it = chat_settings.emplace(std::piecewise_construct, std::forward_as_tuple(request.group_id),
                                        std::forward_as_tuple()).first;
      }
      return &(chat_it->second);
    } else {
      if (chat_it == chat_settings.end()) {
        return &default_chat;
      } else {
        return &(chat_it->second);
      }
    }
  }
  return &default_chat;
}
Game* GameManager::getGame(const GameRequest& request, Chat const& chat) {
  decltype(current_games)::iterator game_it;
  if (request.type != GameRequest::Type::Regist && request.type != GameRequest::Type::AddBit) {
    return &default_game;
  }
  if (request.type == GameRequest::Type::Regist) {
    return current_games.emplace(request.id)
  }
}
