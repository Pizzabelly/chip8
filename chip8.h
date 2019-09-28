#pragma once

#include "util.h"

void vm_step();

typedef struct {
  u8* rom;

  u16 PC;

  u8 SP;
  u16 stack[16];

  u8 DT;
  u8 ST;
  
  u16 I;

  u8 Vx[16];

  u8 keyboard[16];

  u8 screen[0x3F*0x1F];
} chip8_vm;

extern chip8_vm vm;
