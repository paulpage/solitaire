SRC = spider.c
OBJ = ${SRC:.c=.o}

CFLAGS = -Wall
LDFLAGS = -lSDL2 -lSDL2_image

default: spider

.c.o:
	${CC} -c ${CFLAGS} $<

spider: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm -f spider *.o
