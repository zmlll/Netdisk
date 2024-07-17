SRCS:=$(wildcard *.c)
OBJS:=$(SRCS:%.c=%.o)
test:$(OBJS)
	gcc $^ -o $@ -lpthread
%.o:%.c
	gcc -c $^ -o $@ -g
clean:
	$(RM) $(OBJS) test
