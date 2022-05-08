INSTALL_PROGRAM =	install -m 0555

.PHONY: all install

all:

install:
	${INSTALL_PROGRAM} plass ${HOME}/bin
