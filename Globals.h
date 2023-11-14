//
// Created by nikita on 24.08.23.
//

#ifndef CASINO_GLOBALS_H
#define CASINO_GLOBALS_H

#include <mutex>
#include <iostream>
#include <tgbot/tgbot.h>

extern std::mutex tgbot_mutex;

inline constexpr int farm_time = 1800;

inline constexpr int max_bots = 8;

inline constexpr const char* COIN = "\xF0\x9F\x8D\xA9";

inline constexpr const char* OK = "\xE2\x9C\x85";
inline constexpr const char* WARN = "\xE2\x98\x9D\xF0\x9F\x8F\xBB";
inline constexpr const char* FAIL = "\xE2\x9B\x94\xEF\xB8\x8F";
inline constexpr const char* SEARCH = "\xF0\x9F\x94\x8D";

inline constexpr const char* HAHA = "\xF0\x9F\x98\x82";

inline constexpr const char* CASINO = "\xF0\x9F\x8E\xB0";

inline constexpr const char* BOT = "\xF0\x9F\xA4\x96";

inline const std::string str_stage[] = {
    "\xF0\x9F\x98\x84", "\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F", "\xF0\x9F\x98\xAD"};

std::string intToTime(int value);

std::string intToCoins(int64_t value);

std::string  getUserName(TgBot::Bot& bot, int64_t chat_id, int64_t user_id) noexcept;

using uchar = unsigned char;

#endif // CASINO_GLOBALS_H
