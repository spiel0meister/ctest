all: example

%: %.c
	gcc -Wall -Wextra -ggdb -o $@ $<
