SRC = main.c 
OBJ = ${SRC:.c=.o}

CFLAGS = -ansi -pedantic -Wall -ggdb3

all: options ppbase

options:
	@echo ppbase build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: ppbase.h 

ppbase: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f ppbase ${OBJ} 

.PHONY: all options clean 
