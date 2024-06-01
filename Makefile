TESTS=op

.PHONY: test
test: $(foreach test, $(TESTS), tests/$(test))

tests/%: tests/%.c
	gcc -std=c2x -Wall -Wextra -o $@ $^
	./$@
