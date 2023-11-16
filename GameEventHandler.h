//
// Created by nikita on 24.08.23.
//

#ifndef CASINO_GAMEEVENTHANDLER_H
#define CASINO_GAMEEVENTHANDLER_H

#include "Color.h"
#include "Globals.h"
#include "concurrent_queue.h"
#include <memory>

/*
 Regist,        game
 AddBit,        game
 Exit,          -
 GetSettings,   -
 SetBots,       -
 SetBitValue,   -
 * */

struct GameRequest {
  enum class Type : unsigned char {
    Regist,
    AddBit,
    Exit,
    last
  };

  explicit GameRequest(Type t, int64_t user_i = -1, int64_t group_i = -1, int32_t message_i = -1, int64_t bt = -1,
                       Color colo = Color::last, uchar bts = 0) noexcept;

  int64_t user_id;
  int64_t chat_id;
  int64_t own_bit;
  int32_t message_id;
  Type type;
  Color color;
  uchar bts_;
};

using GameEventHandler = concurrent_queue<std::shared_ptr<GameRequest>>;

extern GameEventHandler game_stream;

#endif // CASINO_GAMEEVENTHANDLER_H
