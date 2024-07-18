srcs:=$(wildcard *.c)
exes:=$(srcs:%.c=%)
all:$(exes)
%:%.c
	gcc $^ -o $@ -g -pthread
clean:
	$(RM) $(exes)
