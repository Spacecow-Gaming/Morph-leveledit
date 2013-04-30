CC=clang
CFLAGS=-I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT
LDFLAGS=-L/usr/lib -lSDL -lpthread -lSDL_image -lstdc++
SOURCES=leveledit.cpp

all: leveledit

leveledit:
	$(CC) $(CFLAGS) $(SOURCES) -o bin/leveledit $(LDFLAGS)

clean:
	rm -rf *o bin/leveledit
