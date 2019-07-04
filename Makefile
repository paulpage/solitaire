SRC = cards.c graphics.c
OBJ = ${SRC:.c=.o}

CFLAGS = --std=c99 -Wall -g
LDFLAGS = -lSDL2 -lSDL2_image

default: spider

.c.o:
	${CC} -c ${CFLAGS} $<

spider: ${OBJ} spider.o
	${CC} -o $@ $^ ${LDFLAGS}

clean:
	rm -f spider *.o
