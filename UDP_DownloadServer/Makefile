CC=gcc
CFLAGS=-Wall -Wextra

# Common dependencies 
DEPS=filemanip.h

# Object files for both client and server:
OBJ_CLIENT=UDPclient.c core/filemanip.c
OBJ_SERVER=UDPserver.c core/filemanip.c

UDP-Client: $(OBJ_CLIENT)
	$(CC) -o $@ $^ $(CFLAGS)

UDP-Server: $(OBJ_SERVER)
	$(CC) -o $@ $^ $(CFLAGS)

# Compile both client and server
all: UDP-Client UDP-Server

# Pattern rule for object files, if needed
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# Clean command for cleanup
.PHONY: clean
clean:
	rm -f UDP-Client UDP-Server *.o
