BIN=clienteHTTP
OBJ=cliente.o helper.o socket.o
CC = gcc
#CCFLAGS
all: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $(BIN) $(OBJ)

clean:
	rm *.o
	rm cache
	rm -r cachedir/*
