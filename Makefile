VERSION=	0.1
PROG=		plass
MAN=		plass.1
EXTRA=		README.md Makefile plass-dist.txt

INSTALL=	install
INSTALL_MAN=	${INSTALL} -m 0444
INSTALL_PROGRAM=${INSTALL} -m 0555

PREFIX=		/usr/local
MANDIR=		${PREFIX}/man

# for dist
TMPDIR=		/tmp
STAGEDIR=	${TMPDIR}/plass-${VERSION}

.PHONY: all dist install-local install lint

all:

install-local:
	${INSTALL_PROGRAM} ${PROG} ${HOME}/bin

install:
	${INSTALL_PROGRAM} ${PROG} ${PREFIX}/bin
	${INSTALL_MAN} ${MAN} ${MANDIR}/man1/

lint:
	man -Tlint -l ${MAN}

dist:
	mkdir ${STAGEDIR}
	pax -rw ${PROG} ${MAN} ${EXTRA} ${STAGEDIR}
	tar -C ${TMPDIR} -vzcf plass-${VERSION}.tar.gz plass-${VERSION} | \
		sed -E 's,^plass-${VERSION}/?,,' | \
		sort -u > plass-dist.txt.new
	rm -rf ${STAGEDIR}
	diff -u plass-dist.txt{,.new}
	rm plass-dist.txt.new
