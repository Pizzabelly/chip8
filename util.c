#include <stdlib.h>
#include <stdio.h>

#include "util.h"

u16 hex_str(char* str) {
  return (u16)strtol(str, NULL, 16);
}

u16 hex_char(char c) {
  char buf[2];
  sprintf(buf, "%c", c);
  return (u16)strtol(buf, NULL, 16);
}
