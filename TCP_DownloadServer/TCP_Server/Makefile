CC=gcc
CFLAGS= -Wall -Wextra 
DEPS = 
OBJ = TCPserver.c

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

TCP-Server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm TCP-Server