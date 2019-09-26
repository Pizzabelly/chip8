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

#define ROM_FILE "roms/test_opcode.ch8"

static void load_rom();
static void init_vm();
static void vm_thread(void* v);

chip8_vm vm;

void init_vm() {
  vm.PC = 0x200; // start of chip8 programs
}

void load_rom() {
  memset(&vm, 0, sizeof(chip8_vm));
  FILE* f = fopen(ROM_FILE, "r");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  vm.rom = (u8*)malloc(0xFFF);

  memset(vm.rom, 0, 0xFFF);
  if (fread(&vm.rom[0x200], 1, fsize, f) != fsize) {
    printf("err: failed to read rom fully\n");
    exit(1);
  }

  fclose(f);
}

void vm_thread(void* v) {
  u8 buf[255];
  while (1) {
    // read 2 bytes and convert to big-endian
    u16 in = htons(vm.rom[vm.PC + 0x1]<<8 | vm.rom[vm.PC]);
    //printf("%x\n", in);

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
        switch (in&0xFF) {
          case 0xE0: // CLS
            memset(&vm.screen, 0, sizeof(vm.screen));
            break;
          case 0xEE: // RET
            vm.PC = vm.stack[vm.SP--];
            break;
        } 
        break;
      case 0x1: // JP addr
        vm.PC = (in&0xFFF) - 0x2;
        break;
      case 0x2: // CALL addr
        vm.stack[++vm.SP] = vm.PC;
        vm.PC = (in&0xFFF) - 0x2;
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
        vm.PC = ((in&0xFFF) + vm.Vx[0]) - 0x2;
        break;
      case 0xC: // RND Vx, byte
        vm.Vx[in>>8&0xF] = (rand() % 255) & in&0xFF;
        break;
      case 0xD: // DRW Vx, Vy, nibble
        memcpy(buf, &vm.rom[vm.I], in&0xF);
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
      case 0xE: switch (in&0xFF) {
          case 0x9E: // SKP Vx
            if (vm.keyboard[vm.Vx[in>>8&0xF]]) {
              vm.PC += 0x2;
            }
            break;
          case 0xA1: // SKNP Vx
            if (!vm.keyboard[vm.Vx[in>>8&0xF]]) {
              vm.PC += 0x2;
            }
            break;
        }
        break;
      case 0xF:
        switch (in&0xFF) {
          case 0x07: // LD Vx, DT 
            vm.Vx[in>>8&0xF] = vm.DT;
            break;
          case 0x0A: // LD Vx, K
            vm.Vx[in>>8&0xF] = 0xA;//ui_get_key(true);
            break;
          case 0x15: // LD DT, Vx
            vm.DT = vm.Vx[in>>8&0xF];
            break;
          case 0x18: // LD ST, Vx
            vm.ST = vm.Vx[in>>8&0xF];
            break;
          case 0x1E: // ADD I, Vx
            vm.I += vm.Vx[in>>8&0xF];
            break;
          case 0x29: // LD F, Vx
            // characters
            break;
          case 0x33: // LD B, Vx
            vm.rom[vm.I] = (vm.Vx[in>>8&0xF] / 100);
            vm.rom[vm.I+0x1] = ((vm.Vx[in>>8&0xF] % 100)/10);
            vm.rom[vm.I+0x2] = (vm.Vx[in>>8&0xF] % 10);
            break;
          case 0x55: // LD [I], Vx
            for (int i = 0; i < (in>>8&0xF); i++) {
              vm.rom[(vm.I) + i] = vm.Vx[i];
            }
            break;
          case 0x65: // LD Vx, [I]
            for (int i = 0; i < (in>>8&0xF); i++) {
              vm.Vx[i]  = vm.rom[(vm.I) - i];
            }
            break;
        }
        break;
    }

    vm.PC += 0x2;
    usleep(100);
  }
}

void *dt_thread(void* v) {
  while (1) {
    if (vm.DT > 0) {
      vm.DT--;
    }
    usleep(1000/60);
  }
}

void *st_thread(void* v) {
  while (1) {
    if (vm.ST > 0) {
      vm.ST--;
    }
    usleep(1000/60);
  }
}

int main(void) {
  srand(time(NULL));
  load_rom();
  init_vm();

#ifdef _WIN32
  HANDLE vm = CreateThread(NULL, 0, vm_thread, NULL, 0, NULL);
#else
  pthread_t vm; pthread_create(&vm, NULL, (void*)vm_thread, (void*)0);
  pthread_t dt; pthread_create(&dt, NULL, (void*)dt_thread, (void*)0);
  pthread_t st; pthread_create(&st, NULL, (void*)st_thread, (void*)0);
#endif
  
  //vm_thread(0);
  init_curses();
  curses_thread(0);

  return 0;
}
