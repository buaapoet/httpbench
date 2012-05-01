DEBUG=-g3 -ggdb3
#DEBUG=
NAME=httpbench
#FLAGS=-c -Wall -std=c99 -pedanti
#FLAGS=-std=c99
FLAGS=
all: documentation ctags
	cut -d' ' -f2 debian/changelog | head -n 1 | sed 's/(/#define VERSION "/;s/)/"/' > src/version.h
	gcc $(FLAGS) $(DEBUG) ./src/$(NAME).c -o $(NAME) -lcurl -lpthread
build: all
install:
	test ! -d $(DESTDIR)/usr/bin && mkdir -p $(DESTDIR)/usr/bin || exit 0
	cp $(NAME) $(DESTDIR)/usr/bin
clean:
	test -f ./$(NAME) && rm $(NAME) || exit 0
	test ! -z "$(DESTDIR)" && test -f $(DESTDIR)/usr/bin/$(NAME) && rm $(DESTDIR)/usr/bin/$(NAME) || exit 0
clean-top:
	rm ../$(NAME)_*.tar.gz
	rm ../$(NAME)_*.dsc
	rm ../$(NAME)_*.changes
	rm ../$(NAME)_*.deb
doc: documentation
documentation:
	pod2man --release="$(NAME) $$(cut -d' ' -f2 debian/changelog | head -n 1 | sed 's/(//;s/)//')" \
                       --center="User Commands" ./docs/httpbench.pod > ./docs/httpbench.1
	pod2text ./docs/httpbench.pod > ./docs/httpbench.txt
deb: 
	dpkg-buildpackage
ctags:
	# Generating Source-Tags for Vim 
	ctags `find . -name '*.c'`
