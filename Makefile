VERSION=	0.4
DISTNAME=	plass-${VERSION}
PROGS=		plass pwg totp
MANS=		plass.1 pwg.1 totp.1
EXTRA=		README.md Makefile totp.c

CFLAGS=		-Wall -Wextra
LDFLAGS=	-lcrypto

INSTALL=	install
INSTALL_DATA=	${INSTALL} -m 0644
INSTALL_MAN=	${INSTALL} -m 0444
INSTALL_PROGRAM=${INSTALL} -m 0555

PREFIX=		/usr/local
BINDIR=		${PREFIX}/bin
MANDIR=		${PREFIX}/man

.PHONY: all clean dist install-local install lint mans

all: ${PROGS}

clean:
	rm -f *.o totp

totp: totp.o
	${CC} -o $@ totp.o ${LDFLAGS}

install-local: ${PROGS}
	${INSTALL_PROGRAM} ${PROGS} ${HOME}/bin

install: ${PROGS}
	mkdir -p ${DESTDIR}${BINDIR}
	mkdir -p ${DESTDIR}${MANDIR}/man1
	${INSTALL_PROGRAM} ${PROGS} ${DESTDIR}${BINDIR}
	${INSTALL_MAN} ${MANS} ${DESTDIR}${MANDIR}/man1/

lint:
	man -Tlint -Wstyle -l ${MANS}

.SUFFIXES: .1 .1.html
.1.1.html:
	man -Thtml -Ostyle=mandoc.css -l $< > $@

mans: plass.1.html pwg.1.html totp.1.html

dist: ${DISTNAME}.sha256

${DISTNAME}.sha256: ${DISTNAME}.tar.gz
	sha256 ${DISTNAME}.tar.gz > $@

${DISTNAME}.tar.gz: ${PROGS} ${MANS} ${EXTRA}
	mkdir -p .dist/${DISTNAME}
	${INSTALL_DATA} ${MANS} ${EXTRA} .dist/${DISTNAME}
	${INSTALL_PROGRAM} plass pwg .dist/${DISTNAME}
	cd .dist && tar zcf ../$@ ${DISTNAME}
	rm -rf .dist
