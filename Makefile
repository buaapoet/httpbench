#DEBUG=-g3 -ggdb3
NAME=bench
#FLAGS=-c -Wall -std=c99 -pedanti
#FLAGS=-std=c99
FLAGS=
all:
	gcc $(FLAGS) $(NAME).c -o $(NAME) -lcurl -lpthread
debug:
	gcc $(FLAGS) $(DEBUG) $(NAME).c -o $(NAME) -lcurl -lpthread
clean:
	rm $(NAME)
test: all run-test
run: 
	./run.sh
run-test: 
	./run-test.sh
astyle:
	astyle $(NAME).c
deps:
	sudo apt-get install libcurl4-gnutls-dev
