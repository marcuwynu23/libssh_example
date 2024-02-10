LIBS=libs/ssh.dll
INCLUDE=include
SRC=main.c
BIN=bin/main
CC=gcc

all: $(BIN)

$(BIN): $(SRC)
	$(CC) -o $@ $^ -I$(INCLUDE) -L$(dir $(LIBS)) -l$(notdir $(basename $(LIBS)))
