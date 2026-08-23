CC = gcc
CFLAGS = -Wall -Wextra -g -O2
TARGET = processflow
SOURCE = processos.c
HEADER = processos.h

all: $(TARGET)

$(TARGET): $(SOURCE) $(HEADER)
	$(CC) $(CFLAGS) -o $@ $(SOURCE)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run