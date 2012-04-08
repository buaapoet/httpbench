#DEBUG=-g3 -ggdb3
DEBUG=
NAME=httpbench
#FLAGS=-c -Wall -std=c99 -pedanti
#FLAGS=-std=c99
FLAGS=
all:
	gcc $(FLAGS) $(DEBUG) ./src/$(NAME).c -o $(NAME) -lcurl -lpthread
install:
	test ! -d $(DESTDIR)/usr/bin && mkdir -p $(DESTDIR)/usr/bin || exit 0
	cp $(NAME) $(DESTDIR)/usr/bin
clean:
	test -f ./$(NAME) && rm $(NAME) || exit 0
	test -f ./$(NAME).1 && rm $(NAME).1 || exit 0
	test ! -z "$(DESTDIR)" && test -f $(DESTDIR)/usr/bin/$(NAME) && rm $(DESTDIR)/usr/bin/$(NAME) || exit 0
deb: 
	dpkg-buildpackage
