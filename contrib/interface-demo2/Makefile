CFLAGS=-Wall -g -MD
LDFLAGS=
CC=gcc

TARGET = if_demo

MEMBERS=${TARGET} lib-myconnect lib-mylcd interface af sockets nstrcmp util hw unix inet loopback ether tunnel ppp frame arcnet

DEPS = $(patsubst %,%.d,$(MEMBERS))
OBJECTS = $(patsubst %,%.o,$(MEMBERS))

all: ${TARGET}

${TARGET}: ${OBJECTS}
	${CC} -o ${TARGET} ${OBJECTS} ${CFLAGS} ${LDFLAGS}

clean:
	rm *.o *.d ${TARGET}

-include ${DEPS}

