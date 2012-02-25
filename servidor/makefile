BIN=servidorHTTP
OBJ=servidor.o ../cliente/helper.o
CC = gcc
#CCFLAGS
all: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $(BIN) $(OBJ)

clean:
	rm *.o
