#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#define ROM_FILE "roms/a.ch8"

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
  if (vm.PC > vm.rom_size) {
    return;
  }

  // read 2 bytes and convert to big-endian
  u16 instr = htons(vm.rom[(vm.PC) - 0x1FF]<<8 | vm.rom[(vm.PC) - 0x200]);

  char str[5];
  sprintf(str, "%x", instr);

  switch (*str) {
    case '0':
      if (instr == 0x00E0) {
        //clearscreen
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
      }
  }

  vm.PC += 0x2;
}

int main(void) {
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
      usleep(100);
    }
    return 0;
  } 
  pthread_t t; pthread_create(&t, NULL, (void*)loop, (void*)0);
#endif

  init_curses();
  curses_thread(0);

  return 0;
}
