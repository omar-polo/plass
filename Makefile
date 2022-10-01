VERSION=	0.1
PROG=		plass
DISTNAME=	${PROG}-${VERSION}
MAN=		plass.1
EXTRA=		README.md Makefile

INSTALL=	install
INSTALL_DATA=	${INSTALL} -m 0644
INSTALL_MAN=	${INSTALL} -m 0444
INSTALL_PROGRAM=${INSTALL} -m 0555

PREFIX=		/usr/local
BINDIR=		${PREFIX}/bin
MANDIR=		${PREFIX}/man

.PHONY: all dist install-local install lint

all:

install-local:
	${INSTALL_PROGRAM} ${PROG} ${HOME}/bin

install:
	mkdir -p ${DESTDIR}${BINDIR}
	mkdir -p ${DESTDIR}${MANDIR}/man1
	${INSTALL_PROGRAM} ${PROG} ${DESTDIR}${BINDIR}
	${INSTALL_MAN} ${MAN} ${DESTDIR}${MANDIR}/man1/

lint:
	man -Tlint -l ${MAN}

dist: ${DISTNAME}.sha256

${DISTNAME}.sha256: ${DISTNAME}.tar.gz
	sha256 ${DISTNAME}.tar.gz > $@

${DISTNAME}.tar.gz: ${PROG} ${MAN} ${EXTRA}
	mkdir -p .dist/${DISTNAME}
	${INSTALL_DATA} ${MAN} ${EXTRA} .dist/${DISTNAME}
	${INSTALL_PROGRAM} ${PROG} .dist/${DISTNAME}
	cd .dist && tar zcf ../$@ ${DISTNAME}
	rm -rf .dist
