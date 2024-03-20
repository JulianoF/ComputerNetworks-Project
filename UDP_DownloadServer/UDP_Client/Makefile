CC=gcc
CFLAGS= -Wall -Wextra 
DEPS = 
OBJ = UDPclient.c

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

UDP-Client: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm UDP-Client