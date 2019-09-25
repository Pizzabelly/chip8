#pragma once

#include "util.h"

typedef struct {
  u8* rom;
  u16 rom_size;

  u16 PC;

  u16 stack[16];
  u16 stack_pos;
  u16 stack_count;

  u8 DT;
  u8 ST;
  
  u8 SP;
  u16 I;

  u8 Vx[16];

  u8 keyboard[16];

  u8 screen[64][32];
} chip8_vm;

extern chip8_vm vm;
