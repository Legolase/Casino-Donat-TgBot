//
// Created by nikita on 24.08.23.
//
#include "GameEventHandler.h"
GameRequest::GameRequest(GameRequest::Type t, int64_t user_i, int64_t group_i, int32_t message_i, int64_t bt,
                         Color colo, uchar bts) noexcept
    : user_id(user_i), chat_id(group_i), own_bit(bt), message_id(message_i), type(t), color(colo), bts_(bts) {}

GameEventHandler game_stream;
