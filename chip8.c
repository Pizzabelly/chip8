#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <arpa/inet.h>

#ifdef _WIN32
#include <window.h>
#else
#include <unistd.h>
#include <pthread.h>
#endif

#include "ui.h"
#include "util.h"
#include "chip8.h"

#define ROM_FILE "roms/e.ch8"

static void load_rom();
static void init_vm();
static void push_top_of_stack(u16 val);
static void vm_thread(void* v);

chip8_vm vm;

void init_vm() {
  memset(&vm, 0, sizeof(chip8_vm));
  vm.PC = 0x200; // start of chip8 programs
}

void load_rom() {
  FILE* f = fopen(ROM_FILE, "r");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  vm.rom = malloc(fsize + 1);
  vm.rom_size = 0x200 + fsize;
  if (fread(vm.rom, 1, fsize, f) != fsize) {
    printf("err: failed to read rom fully\n");
    exit(1);
  }

  fclose(f);
}

void push_top_of_stack(u16 val) {
  for (u16 i = vm.SP; i < vm.stack_count; i++) {
    if (i >= 15) {
      exit(1);
    }
    vm.stack[i+1] = vm.stack[i];
  }
  vm.stack[vm.SP] = val;
}

void vm_thread(void* v) {

  // read 2 bytes and convert to big-endian
  u16 in = htons(vm.rom[(vm.PC) - 0x1FF]<<8 | vm.rom[(vm.PC) - 0x200]);

  u8 buf[255];

  if (in == 0xE0) {
    memset(&vm.screen, 0, sizeof(vm.screen));
  }

  // in>>12&0xF  0xFFFf
  //               ^
  // in>>8&0xF   0xFFFf
  //                ^
  // in>>4&0xF   0xFFFf
  //                 ^
  // in&0xF      0xFFFf
  //                  ^
  // in&0xFFF    0xFFFf
  //                ^^^
  // in&0xFF     0xFFFf
  //                 ^^
  // in&0xFF     0xFFFf
  //                 ^^

  switch ((in>>12&0xF)) {
    case 0x0: 
      if (in == 0x00E0) { // CLS
        memset(&vm.screen, 0, sizeof(vm.screen));
      } else if (in == 0x00EE) { // RET
        vm.PC = vm.stack[vm.SP--];
      } 
      break;
    case 0x1: // JP addr
      vm.PC = in&0xFFF;
      break;
    case 0x2: // CALL addr
      vm.SP++;
      push_top_of_stack(vm.PC);
      vm.PC = in&0xFFF;
      break;
    case 0x3: // SE Vx, byte
      if (vm.Vx[in>>8&0xF] == (in&0xFF)) {
        vm.PC += 0x2;
      }
      break;
    case 0x4: // SNE Vx, byte
      if (vm.Vx[in>>8&0xF] != (in&0xFF)) {
        vm.PC += 0x2;
      }
      break;
    case 0x5: // SE Vx, Vy
      if (vm.Vx[in>>8&0xF] == vm.Vx[in>>4&0xF]) {
        vm.PC += 0x2;
      }
      break;
    case 0x6: // LD Vx, byte
      vm.Vx[in>>8&0xF] = in&0xFF;
      break;
    case 0x7: // ADD Vx, byte
      vm.Vx[in>>8&0xF] += in&0xFF;
      break;
    case 0x8: 
      switch(in&0xF) {
        case 0x0: // LD Vx, Vy 
          vm.Vx[in>>8&0xF] = vm.Vx[in>>4&0xF];
          break;
        case 0x1: // OR Vx, Vy 
          vm.Vx[in>>8&0xF] |= vm.Vx[in>>4&0xF];
          break;
        case 0x2: // AND Vx, Vy
          vm.Vx[in>>8&0xF] &= vm.Vx[in>>4&0xF];
          break;
        case 0x3: // XOR Vx, Vy 
          vm.Vx[in>>8&0xF] ^= vm.Vx[in>>4&0xF];
          break;
        case 0x4: // ADD Vx, Vy 
          vm.Vx[in>>8&0xF] += vm.Vx[in>>4&0xF];
          if (vm.Vx[in>>8&0xF] + vm.Vx[in>>4&0xF] > 255) {
            vm.Vx[15] = 1;
            vm.Vx[in>>8&0xF] &= 0xFF;
          } else {
            vm.Vx[15] = 0;
          }
          break;
        case 0x5: // SUB Vx, Vy 
          if (vm.Vx[in>>8&0xF] > vm.Vx[in>>4&0xF]) {
            vm.Vx[15] = 1;
          } else {
            vm.Vx[15] = 0;
          }
          vm.Vx[in>>8&0xF] -= vm.Vx[in>>4&0xF];
          break;
        case 0x6: // SHR Vx {, Vy}
          if ((vm.Vx[in>>8&0xF] & 1) == 1) {
            vm.Vx[15] = 1;
          } else {
            vm.Vx[15] = 0;
          }
          vm.Vx[in>>8&0xF] /= 2;
          break;
        case 0x7: // SUBN Vx, Vy
          if (vm.Vx[in>>8&0xF] < vm.Vx[in>>4&0xF]) {
            vm.Vx[15] = 1;
          } else {
            vm.Vx[15] = 0;
          }
          vm.Vx[in>>8&0xF] = vm.Vx[in>>4&0xF] - vm.Vx[in>>8&0xF];
          break;
        case 0xE: // SHL Vx {, Vy} 
          if ((vm.Vx[in>>8&0xF] & 1) == 1) {
            vm.Vx[15] = 1;
          } else {
            vm.Vx[15] = 0;
          }
          vm.Vx[in>>8&0xF] *= 2;
          break;
      }
      break;
    case 0x9: // SNE Vx, Vy 
      if (vm.Vx[(in>>8&0xF)] != vm.Vx[(in>>4&0xF)]) {
        vm.PC += 0x2;
      }
      break;
    case 0xA: // LD I, addr
      vm.I = in&0xFFF;
      break;
    case 0xB: // JP V0, addr
      vm.PC = (in&0xFFF) + vm.Vx[0];
      break;
    case 0xC: // RND Vx, byte
      vm.Vx[in>>8&0xF] = (rand() % 255) & in&0xFF;
      break;
    case 0xD: // DRW Vx, Vy, nibble
      memcpy(buf, &vm.rom[vm.I - 0x200], in&0xF);
      for (int i = 0; i < (in&0xF); i++) {
        for (int s = 0; s < 8; s++) {
          u8 bit = (buf[i]>>s)&1;

          if (bit) {
            u8 x = (vm.Vx[in>>8&0xF] + 7) - s;
            u8 y = i + vm.Vx[in>>4&0xF];
            
            while (x > 63) { x -= 63; }
            while (y > 31) { y -= 31; }

            if (vm.screen[x][y]) {
              vm.screen[x][y] = 0;
            } else {
              vm.screen[x][y] = 1;
            }
          }
        }
      }
      break;
    case 0xE: 
      switch (in&0xFF) {
        case 0x9E:
          if (vm.keyboard[vm.Vx[in>>8&0xF]]) {
            vm.PC += 0x2;
          }
          break;
        case 0xA1:
          if (!vm.keyboard[vm.Vx[in>>8&0xF]]) {
            vm.PC += 0x2;
          }
          break;
      }
      break;
    case 0xF:
      switch (in&0xFF) {
        case 0x07:
          vm.Vx[in>>8&0xF] = vm.DT;
          break;
        case 0x0A:
          vm.Vx[in>>8&0xF] = ui_get_key(true);
          break;
        case 0x15:
          vm.DT = vm.Vx[in>>8&0xF];
          break;
        case 0x18:
          vm.ST = vm.Vx[in>>8&0xF];
          break;
        case 0x1E:
          vm.I += vm.Vx[in>>8&0xF];
          break;
        case 0x29:
          // characters
          break;
        case 0x33:
          vm.rom[vm.I - 0x200] = floor(vm.Vx[in>>8&0xF] / 100);
          vm.rom[(vm.I - 0x200)+1] = floor((vm.Vx[in>>8&0xF] % 100)/10);
          vm.rom[(vm.I - 0x200)+2] = floor(vm.Vx[in>>8&0xF] % 10);
          break;
        case 0x55:
          for (int i = 0; i < (in>>8&0xF); i++) {
            vm.rom[(vm.I - 0x200) - i] = vm.Vx[i];
          }
          break;
        case 0x65:
          for (int i = 0; i < (in>>8&0xF); i++) {
            vm.Vx[i]  = vm.rom[(vm.I - 0x200) - i];
          }
          break;
      }
  }

  vm.PC += 0x2;
}

int main(void) {
  srand(time(NULL));
  init_vm();
  load_rom();

#ifdef _WIN32
  DWORD WINAPI loop(void* data) {
    while(1) {
      vm_thread(0);
      sleep(1);
    }
    return 0;
  }
  HANDLE thread = CreateThread(NULL, 0, loop, NULL, 0, NULL);
#else
  void *loop(void) {
    while(1) {
      vm_thread(0);
      usleep(1000);
    }
    return 0;
  } 
  pthread_t t; pthread_create(&t, NULL, (void*)loop, (void*)0);
#endif

  init_curses();
  curses_thread(0);

  return 0;
}
