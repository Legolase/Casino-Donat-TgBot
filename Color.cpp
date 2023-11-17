//
// Created by nikita on 24.08.23.
//
#include "Color.h"
#include "Globals.h"

const char* color_text[] = {
    "\xF0\x9F\x94\xB4",
    "\xF0\x9F\x9F\xA2",
    "\xF0\x9F\x94\xB5",
    "\xE2\x9A\xAB\xEF\xB8\x8F",
    "\xE2\x9A\xAA\xEF\xB8\x8F",
    "\xF0\x9F\x9F\xA1",
    "\xF0\x9F\x9F\xA3"
};

inline constexpr int COLOR_SIZE = (sizeof(color_text)/sizeof(color_text[0]));

static_assert(static_cast<int>(Color::last) == COLOR_SIZE);
static_assert(MAX_COLORS == COLOR_SIZE);