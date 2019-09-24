#include <ncurses.h>
#include <string.h>
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
        mvaddstr(x * 2, y, "[]");
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

void curses_thread(void* v) {
  char c;
  while(1) {
    c = getch();
    if (c == 'q') {
      endwin();
      exit(1);
    }
    draw();
    usleep(100);
  }
}
    
  
