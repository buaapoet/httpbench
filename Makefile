#DEBUG=-g3 -ggdb3
DEBUG=
NAME=httpbench
#FLAGS=-c -Wall -std=c99 -pedanti
#FLAGS=-std=c99
FLAGS=
all:
	test ! -d ./dest/usr/bin && mkdir -p ./dest/usr/bin || exit 0
	gcc $(FLAGS) $(DEBUG) ./src/$(NAME).c -o ./dest/usr/bin/$(NAME) -lcurl -lpthread
install:
	test ! -d $(DESTDIR)/usr/bin && mkdir -p $(DESTDIR)/usr/bin || exit 0
	cp ./dest/usr/bin/$(NAME) $(DESTDIR)/usr/bin
clean:
	test -d ./dest && rm -Rf dest || exit 0
	test ! -z "$(DESTDIR)" && test -f $(DESTDIR)/usr/bin/$(NAME) && rm $(DESTDIR)/usr/bin/$(NAME) || exit 0
deb:
	dpkg-buildpackage
