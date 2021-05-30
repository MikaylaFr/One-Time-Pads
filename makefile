#Source: CS261 Assignments
CC = gcc
CFLAGS = -g -Wall -std=gnu99 -g3

all : keygen enc_server enc_client dec_server dec_client

keygen: keygen.o
	$(CC) $(CFLAGS) -o $@ $^

keygen.o : keygen.c

enc_server: enc_server.o
	$(CC) $(CFLAGS) -o $@ $^

enc_server.o : enc_server.c

enc_client: enc_client.o
	$(CC) $(CFLAGS) -o $@ $^

enc_client.o : enc_client.c

dec_client: dec_client.o
	$(CC) $(CFLAGS) -o $@ $^

dec_client.o : dec_client.c

dec_server: dec_server.o
	$(CC) $(CFLAGS) -o $@ $^

dec_server.o : dec_server.c

memTest :
	valgrind --tool=memcheck --leak-check=yes ./enc_server 12345

clean :
	-rm *.o
	-rm keygen enc_server enc_client dec_server dec_client