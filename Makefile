all: CC=gcc
win: CC=x86_64-w64-mingw32-gcc

CFLAGS  = -O3 -Wall -std=c99 $(shell pkg-config --cflags ncursesw)

all: BIN = chip8
win: BIN = chip8.exe

all: executable
win: executable

all: LDFLAGS  = -lncursesw -ltinfow -lpthread
win: LDFLAGS  = ./win/pdcurses.a -lpthread

executable: chip8.c ui.c util.c
	$(CC) $(CFLAGS) -o $(BIN) chip8.c ui.c util.c $(LDFLAGS)

clean:
	$(RM) chip8 
