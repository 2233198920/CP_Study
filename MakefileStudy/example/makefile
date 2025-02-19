
//

.PHONY: clean
EXE = simple

SRCS = $(wildcard *.c)

OBJC = $(SRCS:.c=.o)

$(EXE): $(OBJC)
	g++ $^ -o $@
%.o: %.c
	g++ -o $@ -c $^
clean:
	rm -f $(OBJC) $(EXE)

