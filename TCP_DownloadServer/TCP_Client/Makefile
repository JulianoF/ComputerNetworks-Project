CC=gcc
CFLAGS= -Wall -Wextra -g
DEPS =  
OBJ = TCPclient.c 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

TCP-Client: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm TCP-Client