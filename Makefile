CC      = gcc
CFLAGS  = -Wall -g
LDFLAGS =

PROJECT = sudoku

OBJECTS = main.o

all: $(PROJECT)

$(PROJECT): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm $(OBJECTS) $(PROJECT)
