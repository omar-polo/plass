PROG=		plass
MAN=		plass.1
EXTRA=		README.md Makefile plass-version.mk plass-dist.txt

INSTALL=	install
INSTALL_MAN=	${INSTALL} -m 0444
INSTALL_PROGRAM=${INSTALL} -m 0555

PREFIX=		/usr/local
MANDIR=		${PREFIX}/man

# for dist
TMPDIR=		/tmp
STAGEDIR=	${TMPDIR}/plass-${VERSION}

include plass-version.mk

.PHONY: all dist install lint release

all:

install:
	@if [ '${RELEASE}' = 'Yes' ]; then \
		echo ${INSTALL_PROGRAM} ${PROG} ${PREFIX}/bin; \
		${INSTALL_PROGRAM} ${PROG} ${PREFIX}/bin; \
		echo ${INSTALL_MAN} ${MAN} ${MANDIR}/man1/; \
		${INSTALL_MAN} ${MAN} ${MANDIR}/man1/; \
	else \
		echo ${INSTALL_PROGRAM} ${PROG} ${HOME}/bin; \
		${INSTALL_PROGRAM} ${PROG} ${HOME}/bin; \
	fi

lint:
	man -Tlint -l ${MAN}

release:
	sed -i -e s/^RELEASE=No/RELEASE=Yes/ plass-version.mk
	${MAKE} dist
	sed -i -e s/^RELEASE=Yes/RELEASE=No/ plass-version.mk

dist:
	mkdir ${STAGEDIR}
	pax -rw ${PROG} ${MAN} ${EXTRA} ${STAGEDIR}
	tar -C ${TMPDIR} -vzcf plass-${VERSION}.tar.gz plass-${VERSION} | \
		sed -E 's,^plass-${VERSION}/?,,' | \
		sort -u > plass-dist.txt.new
	rm -rf ${STAGEDIR}
	diff -u plass-dist.txt{,.new}
	rm plass-dist.txt.new
