//
// Created by nikita on 24.08.23.
//

#ifndef CASINO_PERSONEVENTHANDLER_H
#define CASINO_PERSONEVENTHANDLER_H

#include "Color.h"
#include "concurrent_queue.h"
#include <memory>

struct PersonRequest {
  enum class Type : unsigned char {
    AddBit,
    Exit,
    Farm,
    GameRegist,
    GetBalance,
    Help,
    ReturnMoney,
    SpyBalance,
    SetBot,
    SetBit,
    GetBot,
    last
  };

  PersonRequest(Type type, int64_t user_i, int64_t group_i = -1, int32_t message_i = -1, int64_t bt = -1,
                int64_t user_t = -1, Color colo = Color::last) noexcept;

  int64_t user_id_from;
  int64_t user_id_to;
  int64_t chat_id;
  int64_t counter;
  int32_t message_id;
  Type type;
  Color color;
};

using PersonEventHandler = concurrent_queue<std::shared_ptr<PersonRequest>>;

extern PersonEventHandler person_stream;

#endif // CASINO_PERSONEVENTHANDLER_H
