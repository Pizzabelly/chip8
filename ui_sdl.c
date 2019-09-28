#include <time.h>
#include <SDL2/SDL.h>

#include "chip8.h"

#define W 640
#define H 320

SDL_Renderer *r;

void setup() {
  SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);
  SDL_Window* win = SDL_CreateWindow("chip8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, W, H, SDL_WINDOW_SHOWN);
  r = SDL_CreateRenderer(win, -1, SDL_RENDERER_PRESENTVSYNC|SDL_RENDERER_ACCELERATED);  
  SDL_RenderClear(r);
}

void draw() {
  SDL_Rect box = (SDL_Rect){1, 0, 10, 10};
  SDL_SetRenderDrawColor(r, 50, 0, 0, 255);
  SDL_RenderClear(r);
  SDL_SetRenderDrawColor(r, 160, 200, 255, 255);
  //SDL_RenderFillRect(r, &box);
  for (int x = 0; x < 0x3F; x++) {
    for (int y = 0; y < 0x1F; y++) {
      int byte = y * 64 + x;
      int bit = byte & 0x07;
      if ((vm.screen[byte>>3] & (1 << bit)) != 0) {
        box.x = x * 10; box.y = y * 10;
        //SDL_RenderFillRect(r, &box);
        SDL_RenderDrawRect(r, &box);
      }
    }
  }
  SDL_RenderPresent(r);
}

int main(void) {
  srand(time(NULL));
  setup();
  load_rom();
  init_vm();

  int last_60hz = SDL_GetTicks();

  SDL_Event e;
  while (1) {
    SDL_PollEvent(&e);
    switch (e.type) {
      case SDL_KEYDOWN:
        switch (e.key.keysym.sym) {
          case 'q':
            SDL_Quit();
            exit(0);
        }
    }

    int cur = SDL_GetTicks();
    if (cur - last_60hz >= (1000/60)) {
      if (vm.DT > 0) {
        vm.DT--;
      }

      if (vm.ST > 0) {
        vm.ST--;
      }

      draw();
      last_60hz = cur;
    }

    //SDL_Delay(1);
    vm_step();
  }
  return 0;
}
  
