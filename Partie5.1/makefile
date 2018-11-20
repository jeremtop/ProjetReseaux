CC = gcc -Wall -std=gnu11
EXEC = extremite tun64
all: $(EXEC)

extremite: extremite.c
	$(CC) extremite.c -o extremite

tun64: tun64.c
	$(CC) tun64.c -o tun64 
	
clean:
	rm -f *.o  $(EXEC)