#include <libsocket.h>
#include <regex.h>

#include "chat.h"

int main(int argc, char *argv[])
{
	int listenfd, connfd, nready, maxfd;
	struct sockaddr_in serveraddr, clientaddr;
	socklen_t clientlen;
	fd_set rset, allset;
	struct clients clients = {0};
	struct channels channels = {0};
	struct collection collection = {0};

	collection.clients = &clients;
	collection.channels = &channels;

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVER_PORT);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	Bind(listenfd, (struct sockaddr *) &serveraddr, sizeof(struct sockaddr));
	Listen(listenfd, LISTENQ);

	// Setup read set for select
	maxfd = listenfd;
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);

	puts("Server running...");
	for (; ;) {
		clientlen = sizeof(clientaddr);
		// Wait for rset to be ready
		rset = allset;
		nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

		/* If New connection arrived handle it */
		if (FD_ISSET(listenfd, &rset)) {
			puts("New connection initiated...");
			if ((connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen)) == -1) {
				perror("accept error");
			}
			
			/* If client limit reached then don't accept the new connection */
			if (clients.nclient + 1 == FD_SETSIZE) {
				fprintf(stderr, "Warning: Client limit exceeded, rejecting new connection.");
				/* TODO Need to tell client, your connection request rejected */
			}
			
			/* Save new client details in first empty space */
			for (int i = 0; i < FD_SETSIZE; i++) {
				if (clients.clients[i] == NULL) {
					if (server_new_client(&clients, connfd) == -1)
						break;
					FD_SET(connfd, &allset);
					break;
				}
			}
			
			// Increment maxfd if required
			if (connfd > maxfd)
				maxfd = connfd;

			/* If only new connection arrived then skip iteration on
			 * clients below */
			if (--nready == 0)
				continue;
		}

		struct client *tmp;
		/* Find ready socket and handle the client request */
		for (int i = 0; i < clients.clients_i && nready; i++) {
			tmp = clients.clients[i];
			if (tmp == NULL)
				continue;
			/* If client's socket is ready then process it */
			if (FD_ISSET(tmp->fd, &rset)) {
				/* If client terminated then clean client details */
				puts("handle_client_request called..");
				collection.index = i;
				chat_request_handle(&collection, &allset);
				nready--;
			}
		}
	}
}

int server_new_client(struct clients *clients, int sockfd)
{
	struct client *client;	
	if ((client = malloc(sizeof(struct client))) == NULL) {
		perror("server_new_client: malloc error");
		return -1;
	}

	/* Set default values */
	client->fd 					= sockfd;
	client->pair_fd 			= -1;
	client->nick[0] 		= 0;
	client->is_username_set 	= false;
	client->is_assoc 			= false;

	clients->clients[clients->nclient++] = client;
	clients->clients_i++;

	return 0;
}

int	server_del_client(struct clients *clients, int index)
{
	if (clients->clients[index] == NULL)
		return -1;

	free(clients->clients[index]);
	clients->clients[index] = NULL;

	return 0;
}

