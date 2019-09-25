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
  u16 instr = htons(vm.rom[(vm.PC) - 0x1FF]<<8 | vm.rom[(vm.PC) - 0x200]);

  char str[5];
  sprintf(str, "%x", instr);

  u8 buf[255];

  if (instr == 0xE0) {
    memset(&vm.screen, 0, sizeof(vm.screen));
  }

  switch (*str) {
    case '0':
      if (instr == 0x00E0) {
        memset(&vm.screen, 0, sizeof(vm.screen));
      } else if (instr == 0x00EE) {
        vm.PC = vm.stack[vm.SP--];
      } 
      break;
    case '1':
      vm.PC = hex_str(str+1);
      break;
    case '2':
      vm.SP++;
      push_top_of_stack(vm.PC);
      vm.PC = hex_str(str+1);
      break;
    case '3':
      if (vm.Vx[hex_char(str[1])] == hex_str(str+2)) {
        vm.PC += 0x2;
      }
      break;
    case '4':
      if (vm.Vx[hex_char(str[1])] != hex_str(str+2)) {
        vm.PC += 0x2;
      }
      break;
    case '5':
      if (vm.Vx[hex_char(str[1])] == vm.Vx[hex_char(str[2])]) {
        vm.PC += 0x2;
      }
      break;
    case '6':
      vm.Vx[hex_char(str[1])] = hex_str(str+2);
      break;
    case '7':
      vm.Vx[hex_char(str[1])] += hex_str(str+2);
      break;
    case '8':
      switch(str[3]) {
        case '0':
          vm.Vx[hex_char(str[1])] = vm.Vx[hex_char(str[2])];
          break;
        case '1':
          vm.Vx[hex_char(str[1])] |= vm.Vx[hex_char(str[2])];
          break;
        case '2':
          vm.Vx[hex_char(str[1])] &= vm.Vx[hex_char(str[2])];
          break;
        case '3':
          vm.Vx[hex_char(str[1])] ^= vm.Vx[hex_char(str[2])];
          break;
        case '4':
          vm.Vx[hex_char(str[1])] += vm.Vx[hex_char(str[2])];
          if (vm.Vx[hex_char(str[1])] + vm.Vx[hex_char(str[2])] > 255) {
            vm.Vx[15] = 1;
            vm.Vx[hex_char(str[1])] &= 0x255;
          } else {
            vm.Vx[15] = 0;
          }
          break;
        case '5':
          if (vm.Vx[hex_char(str[1])] > vm.Vx[hex_char(str[2])]) {
            vm.Vx[15] = 1;
          } else {
            vm.Vx[15] = 0;
          }
          vm.Vx[hex_char(str[1])] -= vm.Vx[hex_char(str[2])];
          break;
        case '6':
          if ((vm.Vx[hex_char(str[1])] & 1) == 1) {
            vm.Vx[15] = 1;
          } else {
            vm.Vx[15] = 0;
          }
          vm.Vx[hex_char(str[1])] /= 2;
          break;
        case '7':
          if (vm.Vx[hex_char(str[1])] < vm.Vx[hex_char(str[2])]) {
            vm.Vx[15] = 1;
          } else {
            vm.Vx[15] = 0;
          }
          vm.Vx[hex_char(str[1])] = vm.Vx[hex_char(str[2])] - vm.Vx[hex_char(str[1])];
          break;
        case 'e':
          if ((vm.Vx[hex_char(str[1])] & 1) == 1) {
            vm.Vx[15] = 1;
          } else {
            vm.Vx[15] = 0;
          }
          vm.Vx[hex_char(str[1])] *= 2;
          break;
      }
      break;
    case '9':
      if (vm.Vx[hex_char(str[1])] != vm.Vx[hex_char(str[2])]) {
        vm.PC += 0x2;
      }
      break;
    case 'a':
      vm.I = hex_str(str+1);
      break;
    case 'b':
      vm.PC = hex_str(str+1) + vm.Vx[0];
      break;
    case 'c':
      vm.Vx[hex_char(str[1])] = (rand() % 255) & hex_str(str+2);
      break;
    case 'd':
      memcpy(buf, &vm.rom[vm.I - 0x200], hex_char(str[3]));
      for (int i = 0; i < hex_char(str[3]); i++) {
        for (int s = 0; s < 8; s++) {
          u8 bit = (buf[i]>>s)&1;

          if (bit) {
            u8 x = (vm.Vx[hex_char(str[1])] + 7) - s;
            u8 y = i + vm.Vx[hex_char(str[2])];
            
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
    case 'e':
      if (hex_str(str+2) == 0x9E) {
        if (vm.keyboard[vm.Vx[hex_char(str[1])]]) {
          vm.PC += 0x2;
        }
      } else if (hex_str(str+2) == 0xA1) {
        if (!vm.keyboard[vm.Vx[hex_char(str[1])]]) {
          vm.PC += 0x2;
        }
      }
      break;
    case 'f':
      if (hex_str(str+2) == 0x07) {
        vm.Vx[hex_char(str[1])] = vm.DT;
      } else if (hex_str(str+2) == 0x0A) {
        vm.Vx[hex_char(str[1])] = ui_get_key(true);
      } else if (hex_str(str+2) == 0x15) {
        vm.DT = vm.Vx[hex_char(str[1])];
      } else if (hex_str(str+2) == 0x18) {
        vm.ST = vm.Vx[hex_char(str[1])];
      } else if (hex_str(str+2) == 0x1E) {
        vm.I += vm.Vx[hex_char(str[1])];
      } else if (hex_str(str+2) == 0x29) {
        // characters
      } else if (hex_str(str+2) == 0x33) {
        u8 tmp =  vm.Vx[hex_char(str[1])];
        vm.rom[vm.I - 0x200] = floor(tmp / 100);
        vm.rom[(vm.I - 0x200)+1] = floor((tmp % 100)/10);
        vm.rom[(vm.I - 0x200)+2] = floor(tmp % 10);
      } else if (hex_str(str+2) == 0x55) {
        for (int i = 0; i < hex_char(str[1]); i++) {
          vm.rom[(vm.I - 0x200) - i] = vm.Vx[i];
        }
      } else if (hex_str(str+2) == 0x65) {
        for (int i = 0; i < hex_char(str[1]); i++) {
          vm.Vx[i]  = vm.rom[(vm.I - 0x200) - i];
        }
      }
      break;
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
