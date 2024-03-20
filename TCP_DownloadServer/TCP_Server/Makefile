CC=gcc
CFLAGS= -Wall -Wextra -g
DEPS =  ConnectionLib.h 
OBJ = TCPserver.c ConnectionLib.c

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

TCP-Server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm TCP-Server