CC=g++
CFLAGS=-I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT
LDFLAGS=-L/usr/lib -lSDL -lpthread -lSDL_image -lSDL_ttf -lstdc++
SOURCES=src/leveledit.cpp
OBJECTS=$(SOURCES:.cpp=.o)

all: leveledit

leveledit:
	$(CC) $(CFLAGS) $(SOURCES) -o leveledit $(LDFLAGS)

clean:
	rm -rf $(OBJECTS) leveledit
