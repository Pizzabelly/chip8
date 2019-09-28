#include <ncurses.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "chip8.h"
#include "util.h"

void init_curses() {
  initscr();
  curs_set(0);
  noecho();
  nodelay(stdscr, 1);
  clear();
}

void draw() {
  erase();
  for (int x = 0; x < 0x3F; x++) {
    for (int y = 0; y < 0x1F; y++) {
      int byte = y * 64 + x;
      int bit = byte & 0x07;
      if ((vm.screen[byte>>3] & (1 << bit)) != 0) {
        mvaddstr(y, x * 2, "[]");
      }
    }
  }

  char buf[25];
  for (int i = 0; i < 7; i++) {
    sprintf(buf, "V%x: 0x%x", i, vm.Vx[i]);
    mvaddstr(35, 9 * i, buf);
  }
  for (int i = 7; i < 15; i++) {
    sprintf(buf, "V%x: 0x%x", i, vm.Vx[i]);
    mvaddstr(36, 9 * (i - 8), buf);
  }

  sprintf(buf, "Vf: 0x%x", vm.Vx[15]);
  mvaddstr(37, 0, buf);
  sprintf(buf, "SP: 0x%x", vm.SP);
  mvaddstr(37, 9, buf);
  sprintf(buf, "DT: 0x%x", vm.DT);
  mvaddstr(37, 18, buf);
  sprintf(buf, "ST: 0x%x", vm.ST);
  mvaddstr(37, 27, buf);
  sprintf(buf, "PC: 0x%x", vm.PC);
  mvaddstr(37, 36, buf);
  sprintf(buf, "I: 0x%x", vm.I);
  mvaddstr(37, 46, buf);
}

u8 ui_get_key(bool block) {
  do {
    char c = getch();
    switch (c) {
      case '1': return 0x0;
      case '2': return 0x1;
      case '3': return 0x2;
      case '4': return 0x3;
      case 'q': return 0x4;
      case 'w': return 0x5;
      case 'e': return 0x6;
      case 'r': return 0x7;
      case 'a': return 0x8;
      case 's': return 0x9;
      case 'd': return 0xA;
      case 'f': return 0xB;
      case 'z': return 0xC;
      case 'x': return 0xD;
      case 'c': return 0xE;
      case 'v': return 0xF;
    }
  } while (block);
  return 0x10;
}

void curses_thread(void* v) {
  u8 c;
  while(1) {
    draw();
    vm_step();
  
    if (vm.DT > 0) {
      vm.DT--;
      usleep(1000000/60);
    } 

    if (vm.ST > 0) {
      vm.ST--;
      usleep(1000000/60);
    }

    c = getch();
    //if ((c = ui_get_key(false)) <= 0xF) {
    //  vm.keyboard[c] = 1;
    //}
  }
}
    
  
