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
#include <sstream>
#include <string>

// static void appendToBanner(TgBot::Bot& bot, int64_t chat_id, std::stringstream& banner, std::set<int64_t> users,
//                            const char* header, std::string const& win_amount) {
//   banner << header << ' ' << win_amount << "\n";
//
//   // write people winners
//   for (auto id = users.lower_bound(0); id != users.end(); ++id) {
//     banner << "  <b><i>" << getUserName(bot, chat_id, *id) << "</i></b>\n";
//   }
//   // write bots winners
//   for (auto user_id = users.begin(); user_id != users.end() && (*user_id < 0); ++user_id) {
//     banner << "  <i><b>" << NAMES[random(0, NAMES_SIZE - 1)] << BOT << "</b></i>\n";
//   }
//   banner << '\n';
// }

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
    for (int i = bit_counter; i <= bot_in; ++i) {
      addBotBit(bot, static_cast<Color>(random(0, static_cast<int>(Color::last) - 1)));
    }

    std::vector<uchar> places(static_cast<int>(Color::last));
    for (int i = 0; i < places.size(); ++i) {
      places[i] = i;
    }
    std::sort(places.begin(), places.end(), [&](uchar a, uchar b) { return goals[a].size() > goals[b].size(); });

    while (!places.empty() && goals[places.back()].empty()) {
      places.pop_back();
    }

    std::vector<std::vector<uchar>> equal_colors{{places[0]}};
    for (int i = 1; i < places.size(); ++i) {
      if (goals[places[i]].size() != goals[equal_colors.back().back()].size()) {
        equal_colors.emplace_back();
      }
      equal_colors.back().push_back(places[i]);
    }
    std::stringstream banner_result;
    banner_result << "<b><i>" << CASINO << " Результаты</i></b>:\n\n";
    int num = 0;
    char group_stage = -1;
    int64_t win, group_bits, real_win;
    double win_percent;
    for (int i = equal_colors.size() - 1, j = 0; i >= 0; --i, ++j) {
      if (i != j) {
        group_bits = equal_colors[i].size() * goals[equal_colors[i][0]].size() +
                     equal_colors[j].size() * goals[equal_colors[j][0]].size();
        win_percent = static_cast<double>(goals[equal_colors[j][0]].size()) /
                      (goals[equal_colors[i][0]].size() + goals[equal_colors[j][0]].size());
        win = (win_percent * (group_bits * own_bit)) / (equal_colors[i].size() * goals[equal_colors[i][0]].size());
      }
      else {
        win = own_bit;
      }

      // checking time, when winners get lower win
      if ((i > j && win < own_bit) || (i < j && win > own_bit)) win = own_bit;

      real_win = win - own_bit;
      if (group_stage == -1) {
        group_stage = (real_win > 0) ? 0 : ((real_win == 0) ? 1 : 2);
        banner_result << str_stage[group_stage] << '\n';
      }
      else if (group_stage == 0 && real_win <= 0) {
        group_stage = (real_win == 0) ? 1 : 2;
        banner_result << str_stage[group_stage] << '\n';
      }
      else if (group_stage < 2 && real_win < 0) {
        group_stage = 2;
        banner_result << str_stage[group_stage] << '\n';
      }
      std::lock_guard lg(tgbot_mutex);
      for (auto const& colore : equal_colors[i]) {
        // show people
        for (auto user_id = goals[colore].lower_bound(0); user_id != goals[colore].end(); ++user_id) {
          person_stream.push(
              std::make_shared<PersonRequest>(PersonRequest::Type::ReturnMoney, *user_id, group_id, message_id, win));
          banner_result << "  " << ++num << ") " << color_text[colore] << ' ';
          try {
            auto user = bot.getApi().getChatMember(group_id, *user_id)->user;
            banner_result << "<b>@" << user->username << "</b> ";
          } catch (...) {
            banner_result << "<i>Unknown</i> ";
          }
          banner_result << ((real_win < 0) ? '-' : '+') << intToCoins(std::abs(real_win)) << '\n';
        }
        // show bots
        for (auto user_id = goals[colore].begin(); user_id != goals[colore].end() && (*user_id < 0); ++user_id) {
          banner_result << "  " << std::to_string(++num) << ") " << color_text[colore] << " <i><b>"
                        << NAMES[random(0, NAMES_SIZE - 1)] << BOT << "</b></i> " << ((real_win < 0) ? '-' : '+')
                        << intToCoins(std::abs(real_win)) << '\n';
        }
      }
    }
    std::lock_guard lg(tgbot_mutex);
    bot.getApi().sendMessage(group_id, banner_result.str(), true, 0, nullptr, "HTML");
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
