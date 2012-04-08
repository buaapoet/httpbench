#DEBUG=-g3 -ggdb3
DEBUG=
NAME=httpbench
#FLAGS=-c -Wall -std=c99 -pedanti
#FLAGS=-std=c99
FLAGS=
all: documentation
	gcc $(FLAGS) $(DEBUG) ./src/$(NAME).c -o $(NAME) -lcurl -lpthread
install:
	test ! -d $(DESTDIR)/usr/bin && mkdir -p $(DESTDIR)/usr/bin || exit 0
	cp $(NAME) $(DESTDIR)/usr/bin
clean:
	test -f ./$(NAME) && rm $(NAME) || exit 0
	test ! -z "$(DESTDIR)" && test -f $(DESTDIR)/usr/bin/$(NAME) && rm $(DESTDIR)/usr/bin/$(NAME) || exit 0
documentation:
	pod2man --release="$(NAME) $$(cut -d' ' -f2 debian/changelog | head -n1 | sed 's/(//;s/)//')" \
                       --center="User Commands" ./docs/httpbench.pod > ./docs/httpbench.1
	pod2text ./docs/httpbench.pod > ./docs/httpbench.txt
deb: 
	dpkg-buildpackage
