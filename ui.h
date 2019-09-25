#pragma once

#include <stdbool.h>
#include "util.h"

void curses_thread(void* v);
u8 ui_get_key(bool block);
void init_curses();
