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
  for (int x = 0; x < 64; x++) {
    for (int y = 0; y < 32; y++) {
      if (vm.screen[x][y]) {
        mvaddstr(y, x * 2, "[]");
      }
    }
  }

  char buf[25];
  for (int i = 0; i < 7; i++) {
    sprintf(buf, "V%x: 0x%x", i, vm.Vx[i]);
    mvaddstr(34, 9 * i, buf);
  }
  for (int i = 7; i < 15; i++) {
    sprintf(buf, "V%x: 0x%x", i, vm.Vx[i]);
    mvaddstr(35, 9 * (i - 8), buf);
  }
  sprintf(buf, "Vf: 0x%x", vm.Vx[15]);
  mvaddstr(36, 0, buf);
  sprintf(buf, "SP: 0x%x", vm.SP);
  mvaddstr(36, 9, buf);
  sprintf(buf, "DT: 0x%x", vm.DT);
  mvaddstr(36, 18, buf);
  sprintf(buf, "ST: 0x%x", vm.ST);
  mvaddstr(36, 27, buf);
  sprintf(buf, "PC: 0x%x", vm.PC);
  mvaddstr(36, 36, buf);
  sprintf(buf, "I: 0x%x", vm.I);
  mvaddstr(36, 46, buf);
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
}

void curses_thread(void* v) {
  while(1) {
    erase();
    draw();
    vm.keyboard[ui_get_key(false)] = 1;
    usleep(100);
  }
}
    
  
