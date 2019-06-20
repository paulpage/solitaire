SRC = cards.c graphics.c
OBJ = ${SRC:.c=.o}

CFLAGS = -Wall -g -Wextra
LDFLAGS = -lSDL2 -lSDL2_image

default: spider

.c.o:
	${CC} -c ${CFLAGS} $<

example: ${OBJ} example.o
	${CC} -o $@ $^ ${LDFLAGS}

spider: ${OBJ} spider.o
	${CC} -o $@ $^ ${LDFLAGS}

clean:
	rm -f spider *.o
