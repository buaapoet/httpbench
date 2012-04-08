#DEBUG=-g3 -ggdb3
NAME=httpbench
#FLAGS=-c -Wall -std=c99 -pedanti
#FLAGS=-std=c99
FLAGS=
all:
	gcc $(FLAGS) $(NAME).c -o $(NAME) -lcurl -lpthread
debug:
	gcc $(FLAGS) $(DEBUG) $(NAME).c -o $(NAME) -lcurl -lpthread
clean:
	rm $(NAME) *.deb 2>/dev/null
test: all run-test
astyle:
	astyle $(NAME).c
	rm *.orig
deps:
	sudo apt-get install libcurl4-gnutls-dev
   
