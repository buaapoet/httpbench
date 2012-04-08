#DEBUG=-g3 -ggdb3
NAME=httpbench
#FLAGS=-c -Wall -std=c99 -pedanti
#FLAGS=-std=c99
FLAGS=
all:
	test ! -d ./dest/usr/bin && mkdir -p ./dest/usr/bin || exit 0
	gcc $(FLAGS) ./src/$(NAME).c -o ./dest/usr/bin/$(NAME) -lcurl -lpthread
install:
	test ! -d $(DESTDIR)/usr/bin && mkdir -p $(DESTDIR)/usr/bin || exit 0
	cp ./dest/usr/bin/$(NAME) $(DESTDIR)/usr/bin
debug:
	gcc $(FLAGS) $(DEBUG) ./src/$(NAME).c -o $(NAME) -lcurl -lpthread
clean:
	test -d ./dest && rm -Rf dest || exit 0
deb:
	dpkg-buildpackage
