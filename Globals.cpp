//
// Created by nikita on 25.08.23.
//
#include "Globals.h"

std::mutex tgbot_mutex;

std::string intToTime(int value) {
  std::string result;
  bool written = false;
  if (!(written && value == 0) && value >= 3600) {
    result += std::to_string(value / 3600) + " час ";
    value %= 3600;
    written = true;
  }
  if (!(written && value == 0) && value >= 60) {
    result += std::to_string(value / 60) + " мин ";
    value %= 60;
    written = true;
  }
  if (!(written && value == 0) && value >= 0) {
    result += std::to_string(value) + " сек";
    written = true;
  }
  return result;
}

std::string intToCoins(int64_t value) {
  return std::to_string(value) + " " + COIN;
}

std::string getUserName(TgBot::Bot& bot, int64_t chat_id, int64_t user_id) noexcept {
  try {
    auto user = bot.getApi().getChatMember(chat_id, user_id)->user;
    if (user->username.empty()) {
      return "<u>" + user->firstName + "</u>";
    } else {
      return '@' + user->username;
    }
  } catch (...) {
    return "Unknown";
  }
}
