#DEBUG=-g3 -ggdb3
NAME=httpbench
#FLAGS=-c -Wall -std=c99 -pedanti
#FLAGS=-std=c99
FLAGS=
all:
	sh -c '[ ! -d ./dest/usr/bin ] && mkdir -p ./dest/usr/bin'
	gcc $(FLAGS) ./src/$(NAME).c -o ./dest/usr/bin/$(NAME) -lcurl -lpthread
install:
	sh -c '[ ! -d $(DESTDIR)/usr/bin ] && mkdir -p $(DESTDIR)/usr/bin'
	cp ./dest/usr/bin/$(NAME) $(DESTDIR)/usr/bin
debug:
	gcc $(FLAGS) $(DEBUG) ./src/$(NAME).c -o $(NAME) -lcurl -lpthread
clean:
	sh -c '[ -e ./dest ] && rm -Rf dest'
astyle:
	astyle $(NAME).c
	rm *.orig
deb:
	dpkg-buildpackage
lint:
	lintian ../*.deb
