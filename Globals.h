//
// Created by nikita on 24.08.23.
//

#ifndef CASINO_GLOBALS_H
#define CASINO_GLOBALS_H

#include <iostream>
#include <mutex>
#include <tgbot/tgbot.h>

using uchar = unsigned char;

extern std::mutex tgbot_mutex;

inline constexpr int64_t MINIMUM_BIT = 20;
inline constexpr int64_t MAX_FARM_AMOUNT = MINIMUM_BIT * 3;
inline constexpr int64_t MIN_FARM_AMOUNT = MAX_FARM_AMOUNT * 0.30;
inline constexpr int DEFAULT_BOTS = 4;
inline constexpr int MIN_COLORS = 3;
inline constexpr int DEFAULT_COLORS = 5;
inline constexpr int MAX_COLORS = 7;
inline constexpr int64_t MAX_BALANCE = 1'000'000'000;

// controll number of colors
inline constexpr uchar MAX_ACTIVE_BITS = 1;

inline constexpr int farm_time = 1800;

inline constexpr int max_bots = 8;

inline constexpr const char* COIN = "\xF0\x9F\x8D\xA9";

inline constexpr const char* OK = "\xE2\x9C\x85";
inline constexpr const char* WARN = "\xE2\x98\x9D\xF0\x9F\x8F\xBB";
inline constexpr const char* FAIL = "\xE2\x9B\x94\xEF\xB8\x8F";
inline constexpr const char* SEARCH = "\xF0\x9F\x94\x8D";

inline constexpr const char* HAHA = "\xF0\x9F\x98\x82";
inline constexpr const char* SHHH = "\xF0\x9F\xA4\xAD";

inline constexpr const char* CASINO = "\xF0\x9F\x8E\xB0";

inline constexpr const char* BOT = "\xF0\x9F\xA4\x96";

inline const char* str_stage[] = {"\xF0\x9F\x98\x84 <u>Победа</u>",
                                  "\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F <u>Ничья</u>",
                                  "\xF0\x9F\x98\xAD <u>Поражение</u>"};

inline constexpr int MAX_MULTIPLIER = 3;

std::string intToTime(int value);

std::string intToCoins(int64_t value);

std::string getUserName(TgBot::Bot& bot, int64_t chat_id, int64_t user_id) noexcept;

#endif // CASINO_GLOBALS_H
