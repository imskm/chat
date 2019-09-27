CC=gcc
LIBSPATH=~/Dropbox/Developments/c_devl/clib/objs
CFLAGS=-I ~/Dropbox/Developments/c_devl/clib/headers/
LIBS=$(LIBSPATH)/libsocket.o $(LIBSPATH)/str.o 
CFLAGS+=-g

server: server.c
	$(CC) -o server server.c $(LIBS) $(CFLAGS)

client: client.c
	$(CC) -o client client.c $(LIBS) $(CFLAGS)

