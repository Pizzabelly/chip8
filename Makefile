all: CC=gcc
win: CC=x86_64-w64-mingw32-gcc

CFLAGS  = -O3 -Wall -std=c99 $(shell pkg-config --cflags sdl2)

all: BIN = chip8
win: BIN = chip8.exe

all: executable
win: executable

all: LDFLAGS  = -lSDL2
win: LDFLAGS  = -lSDL2

executable: chip8.c ui_sdl.c
	$(CC) $(CFLAGS) -o $(BIN) chip8.c ui_sdl.c $(LDFLAGS)

clean:
	$(RM) chip8 
