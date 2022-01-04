SRC = graphics.c
OBJ = ${SRC:.c=.o}

CFLAGS = -Wall -g `pkg-config --cflags --libs sdl2 gl glew`
LDFLAGS = `pkg-config --libs sdl2 gl glew` -lm

default: freecell

.c.o:
	${CC} -c ${CFLAGS} $<

freecell: ${OBJ} main.o
	${CC} -o $@ $^ ${LDFLAGS}

clean:
	rm -f freecell *.o

run: freecell
	./freecell
