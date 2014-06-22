#DEBUG=-g3 -ggdb3
DEBUG=
NAME=httpbench
#FLAGS=-c -Wall -std=c99 -pedanti
#FLAGS=-std=c99
FLAGS=
all: version documentation
	cut -d' ' -f2 debian/changelog | head -n 1 | sed 's/(/#define VERSION "/;s/)/"/' > src/version.h
	gcc $(FLAGS) $(DEBUG) ./src/$(NAME).c -o $(NAME) -lcurl -lpthread
build: all
install:
	test ! -d $(DESTDIR)/usr/bin && mkdir -p $(DESTDIR)/usr/bin || exit 0
	cp $(NAME) $(DESTDIR)/usr/bin
clean:
	test -f ./$(NAME) && rm $(NAME) || exit 0
	test ! -z "$(DESTDIR)" && test -f $(DESTDIR)/usr/bin/$(NAME) && rm $(DESTDIR)/usr/bin/$(NAME) || exit 0
documentation:
	pod2man --release="$(NAME) $$(cut -d' ' -f2 debian/changelog | head -n 1 | sed 's/(//;s/)//')" \
                       --center="User Commands" ./docs/httpbench.pod > ./docs/httpbench.1
	pod2text ./docs/httpbench.pod > ./docs/httpbench.txt
	# For github page
	cp ./docs/$(NAME).pod README.pod
release: dch deb dput
	bash -c "git tag $$(cat .version)"
	git push --tags
	git commit -a -m 'New release'
	git push origin master
version:
	cut -d' ' -f2 debian/changelog | head -n 1 | sed 's/(//;s/)//' > .version
dch: 
	dch -i
deb: 
	dpkg-buildpackage
dput:
	dput -u wheezy-buetowdotorg ../$(NAME)_$$(cat ./.version)_amd64.changes
