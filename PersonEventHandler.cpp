//
// Created by nikita on 24.08.23.
//

#include "PersonEventHandler.h"
PersonRequest::PersonRequest(PersonRequest::Type t, int64_t user_i, int64_t group_i, int32_t message_i, int64_t bt,
                             int64_t user_t, Color colo) noexcept
    : type(t), user_id_from(user_i), user_id_to(user_t), group_id(group_i), message_id(message_i), counter(bt),
      color(colo) {}

PersonEventHandler person_stream;
