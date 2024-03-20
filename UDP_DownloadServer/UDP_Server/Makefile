CC=gcc
CFLAGS= -Wall -Wextra 
DEPS = 
OBJ = UDPserver.c

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

UDP-Server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm UDP-Server