CC=gcc
CFLAGS= -Wall -Wextra 
DEPS =  ConnectionLib.h FileLib.h
OBJ = TCPserver.c ConnectionLib.c FileLib.c

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

TCP-Server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm TCP-Server