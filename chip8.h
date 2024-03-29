#pragma once

#include "util.h"

void init_vm(void);
void vm_step(void);
void load_rom(char *path);

typedef struct {
  u8* ram;

  u16 PC;

  u8 SP;
  u16 stack[16];

  u8 DT;
  u8 ST;
  
  u16 I;

  u8 Vx[16];

  u8 keyboard[16];

  u8 screen[250];
} chip8_vm;

extern chip8_vm vm;
