# hlock - simple screen locker
# See LICENSE file for copyright and license details.

include config.mk

SRC = hlock.c
OBJ = ${SRC:.c=.o}

all: options hlock

options:
	@echo hlock build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

hlock: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f hlock ${OBJ} hlock-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p hlock-${VERSION}
	@cp -R LICENSE Makefile README config.mk ${SRC} hlock-${VERSION}
	@tar -cf hlock-${VERSION}.tar hlock-${VERSION}
	@gzip hlock-${VERSION}.tar
	@rm -rf hlock-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f hlock ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/hlock
	@chmod u+s ${DESTDIR}${PREFIX}/bin/hlock

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/hlock

.PHONY: all options clean dist install uninstall
