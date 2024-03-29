#define SDL_MAIN_HANDLED

#include <time.h>
#include <stdbool.h>

#ifdef _WIN32
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

#include "chip8.h"

#define W 640
#define H 320

SDL_Renderer *r;

void setup(void) {
  SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);
  SDL_Window* win = SDL_CreateWindow("chip8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, W, H, SDL_WINDOW_SHOWN);
  r = SDL_CreateRenderer(win, -1, SDL_RENDERER_PRESENTVSYNC|SDL_RENDERER_ACCELERATED);  
  SDL_RenderClear(r);
}

void draw(void) {
  SDL_Rect box = (SDL_Rect){0, 0, 10, 10};
  SDL_SetRenderDrawColor(r, 12, 5, 9, 255);
  SDL_RenderClear(r);
  SDL_SetRenderDrawColor(r, 228, 167, 104, 255);
  for (int x = 0; x <= 0x3F; x++) {
    for (int y = 0; y <= 0x1F; y++) {
      int byte = y * 64 + x;
      int bit = byte & 0x07;
      if ((vm.screen[byte>>3] & (1 << bit)) != 0) {
        box.x = x * 10; box.y = y * 10;
        SDL_RenderFillRect(r, &box);
        SDL_RenderDrawRect(r, &box);
      }
    }
  }
  SDL_RenderPresent(r);
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("./%s <rom file>\n", argv[0]);
    return 1;
  }

  srand(time(NULL));

  load_rom(argv[1]);
  init_vm();
  setup();

  int last_500hz = SDL_GetTicks();
  int last_60hz = last_500hz;
  int last_45hz = last_500hz;

  bool quit = false;

  SDL_Event e;
  while (!quit) {
    SDL_PollEvent(&e);
    switch (e.type) {
      case SDL_QUIT:
        quit = true; break;
      case SDL_KEYDOWN:
        switch (e.key.keysym.sym) {
          case SDLK_ESCAPE: quit = true; break;
          case '1': vm.keyboard[0x0] = 1; break;
          case '2': vm.keyboard[0x1] = 1; break;
          case '3': vm.keyboard[0x2] = 1; break;
          case '4': vm.keyboard[0x3] = 1; break;
          case 'q': vm.keyboard[0x4] = 1; break;
          case 'w': vm.keyboard[0x5] = 1; break;
          case 'e': vm.keyboard[0x6] = 1; break;
          case 'r': vm.keyboard[0x7] = 1; break;
          case 'a': vm.keyboard[0x8] = 1; break;
          case 's': vm.keyboard[0x9] = 1; break;
          case 'd': vm.keyboard[0xA] = 1; break;
          case 'f': vm.keyboard[0xB] = 1; break;
          case 'z': vm.keyboard[0xC] = 1; break;
          case 'x': vm.keyboard[0xD] = 1; break;
          case 'c': vm.keyboard[0xE] = 1; break;
          case 'v': vm.keyboard[0xF] = 1; break;
        }
        break;
      case SDL_KEYUP:
        switch (e.key.keysym.sym) {
          case '1': vm.keyboard[0x0] = 0; break;
          case '2': vm.keyboard[0x1] = 0; break;
          case '3': vm.keyboard[0x2] = 0; break;
          case '4': vm.keyboard[0x3] = 0; break;
          case 'q': vm.keyboard[0x4] = 0; break;
          case 'w': vm.keyboard[0x5] = 0; break;
          case 'e': vm.keyboard[0x6] = 0; break;
          case 'r': vm.keyboard[0x7] = 0; break;
          case 'a': vm.keyboard[0x8] = 0; break;
          case 's': vm.keyboard[0x9] = 0; break;
          case 'd': vm.keyboard[0xA] = 0; break;
          case 'f': vm.keyboard[0xB] = 0; break;
          case 'z': vm.keyboard[0xC] = 0; break;
          case 'x': vm.keyboard[0xD] = 0; break;
          case 'c': vm.keyboard[0xE] = 0; break;
          case 'v': vm.keyboard[0xF] = 0; break;
        }
        break;
    }

    int cur = SDL_GetTicks();
  
    if (cur - last_500hz >= (1000/500)) {
      vm_step();
      last_500hz = cur;
    }

    if (cur - last_60hz >= (1000/60)) {
      if (vm.DT > 0) {
        vm.DT--;
      }

      if (vm.ST > 0) {
        vm.ST--;
      }

      last_60hz = cur;
    }

    if (cur - last_45hz >= (1000/45)) {
      draw();
      last_45hz = cur;
    }
  }

  SDL_Quit();

  return 0;
}
  
