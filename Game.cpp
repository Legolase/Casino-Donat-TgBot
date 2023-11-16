//
// Created by nikita on 24.08.23.
//

#include "Game.h"
#include "Globals.h"
#include "Names.h"
#include "PersonEventHandler.h"
#include "rand.h"
#include <algorithm>
#include <cmath>
#include <string>

static void appendToBanner(TgBot::Bot& bot, int64_t char_id, std::string& banner, std::set<int64_t> users,
                           const char* header, std::string const& win_amount) {
  banner = banner + header + ' ' + win_amount + "\n";

  // write people winners
  for (auto id = users.lower_bound(0); id != users.end(); ++id) {
    banner = banner + "  <b><i>" + getUserName(bot, char_id, *id) + "</i></b>\n";
  }
  // write bots winners
  for (auto user_id = users.begin(); user_id != users.end() && (*user_id < 0); ++user_id) {
    banner = banner + "  <i><b>" + NAMES[random(0, NAMES_SIZE - 1)] + BOT + "</b></i>\n";
  }
  banner += "\n";
}

bool Game::update(TgBot::Bot& bot, int64_t group_id) {
  if (lifetime() < MAX_LIFETIME) {
    return false;
  }
  {
    std::lock_guard lg(tgbot_mutex);
    try {
      //      bot.getApi().unpinChatMessage(chat_id, message_id);
      for (auto const& mess : bit_message) {
        try {
          bot.getApi().deleteMessage(mess.first, mess.second);
        } catch (...) {
        }
      }
      bot.getApi().deleteMessage(group_id, message_id);
    } catch (...) {
    }
  }

  if (bit_counter > 0) {
    // bots set bits
    for (int i = bit_counter; i <= bot_in; ++i) {
      addBotBit(bot, static_cast<Color>(random(0, static_cast<int>(Color::last) - 1)));
    }

    // chosing win color
    std::vector<int> voted_colors;
    voted_colors.reserve(static_cast<int>(Color::last));
    for (int i = 0; i < goals.size(); ++i) {
      if (!goals[i].empty()) {
        voted_colors.push_back(i);
      }
    }
    int winner_color = voted_colors[random(0, voted_colors.size() - 1)];
    int64_t const win_amount = (own_bit * bit_counter) / goals[winner_color].size();

    // give rewards
    int64_t temp;
    for (auto color : voted_colors) {
      temp = (color == winner_color) ? win_amount : 0;
      for (auto id = goals[color].lower_bound(0); id != goals[color].end(); ++id) {
        person_stream.push(
            std::make_shared<PersonRequest>(PersonRequest::Type::ReturnMoney, *id, group_id, message_id, temp));
      }
    }

    // make banner
    std::lock_guard lg(tgbot_mutex);
    std::string banner_result =
        "<b><i>" + std::string(CASINO) + " Результаты</i></b>:\n\n <u>Победный цвет</u>: " + color_text[winner_color] + "\n\n";

    appendToBanner(bot, group_id, banner_result, goals[winner_color], color_text[winner_color],
                   "+" + intToCoins(win_amount));

    std::string lost_amount = intToCoins(-own_bit);
    for (auto color : voted_colors) {
      if (color != winner_color) {
        appendToBanner(bot, group_id, banner_result, goals[color], color_text[color], lost_amount);
      }
    }
    banner_result = banner_result + "\n<i>Новая игра:</i> /newgame";
    bot.getApi().sendMessage(group_id, banner_result, false, 0, nullptr, "HTML");
  }
  return true;
}
int Game::lifetime() const noexcept {
  return time() - start_time;
}
Game::Game(int32_t ms_id, int64_t bt, int bots_in) : message_id(ms_id), own_bit(bt), bot_in(bots_in) {}

void Game::addBit(TgBot::Bot& bot, const GameRequest& req) {
  goals[static_cast<uchar>(req.color)].insert(req.user_id);
  ++bit_counter;
  std::lock_guard lg(tgbot_mutex);
  TgBot::Message::Ptr msg;
  try {
    auto user = bot.getApi().getChatMember(req.chat_id, req.user_id)->user;
    if (user->username.size() > 0) {
      msg = bot.getApi().sendMessage(req.chat_id, std::string("Ваша (@") + user->username + ") ставка принята.");
    }
    else {
      TgBot::MessageEntity::Ptr entity(new TgBot::MessageEntity);
      entity->type = TgBot::MessageEntity::Type::TextMention;
      entity->user = user;
      entity->offset = 0;
      entity->length = 4;
      msg = bot.getApi().sendMessage(req.chat_id, "Ваша ставка принята.", false, 0, nullptr, "", false, {entity});
    }
  } catch (...) {
    msg = bot.getApi().sendMessage(req.chat_id, "<i>Unknown</i>. Ваша ставка принята.", false, 0, nullptr, "HTML");
  }
  bit_message.emplace_back(msg->chat->id, msg->messageId);
}

void Game::addBotBit(TgBot::Bot& bot, Color col) {
  auto& it = goals[static_cast<uchar>(col)];
  ++bit_counter;
  if (it.empty()) {
    it.insert(-1);
    return;
  }
  it.insert(-static_cast<int64_t>(it.size() + 1));
}
