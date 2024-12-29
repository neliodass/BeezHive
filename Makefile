CC = gcc
CFLAGS = -Wall -Wextra -std=c99
SRCS = beekeeper.c queenBee.c workerBee.c

# Pliki nagłówkowe
HEADERS = hive_manager.h
OBJS = $(SRCS:.c=.o)

# Pliki wykonywalne
TARGETS = beekeeper queenBee workerBee

all: $(TARGETS)

beekeeper: beekeeper.o
	$(CC) $(CFLAGS) -o $@ $^

queenBee: queenBee.o
	$(CC) $(CFLAGS) -o $@ $^

workerBee: workerBee.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OBJS) $(TARGETS)
