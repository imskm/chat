CC=gcc
LIBSPATH=~/Dropbox/Developments/c_devl/clib/objs
CFLAGS=-I ~/Dropbox/Developments/c_devl/clib/headers/
LIBS=$(LIBSPATH)/libsocket.o $(LIBSPATH)/str.o chat.o command.o request.o response.o
CFLAGS+=-g

server: server.c
	$(CC) -o server server.c $(LIBS) $(CFLAGS)

client: client.c
	$(CC) -o client client.c $(LIBS) $(CFLAGS)

command: command.c
	$(CC) -c -o command.o command.c $(CFLAGS)

chat: chat.c
	$(CC) -c -o chat.o chat.c $(CFLAGS)

request: request.c
	$(CC) -c -o request.o request.c $(CFLAGS)
response: response.c
	$(CC) -c -o response.o response.c $(CFLAGS)

clean:
	rm *.o
	rm client
	rm server
