#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "chip8.h"

void init_curses() {
  initscr();
  curs_set(0);
  noecho();
  nodelay(stdscr, 1);
  clear();
}

void curses_thread(void* v) {
  char c;
  while(1) {
    c = getch();
    if (c == 'q') {
      endwin();
      exit(1);
    }
    mvaddstr(0, 0, "V0  V1  V2  V3  V4  V5  V6  V7  V8  V9  VA  VB  VC  VD  VE  VF");
    char buf[5];
    for (int i = 0; i < 16; i++) {
      sprintf(buf, "%i", vm.Vx[i]);
      mvaddstr(1, i * 4, buf);
    }
    usleep(100);
  }
}
    
  
