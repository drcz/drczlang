CC=gcc
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=parser.c SE.c srecd.c main.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=srecd

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@