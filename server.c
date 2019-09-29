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
				if (server_handle_request(&clients, i) == 0) {
					printf("[*] Client <%s> terminated...\n", tmp->nick);
					FD_CLR(tmp->fd, &allset);
					close(tmp->fd);
					server_del_client(&clients, i);
				}
				nready--;
			}
		}


	}

}

int server_handle_request(struct clients *clients, int index)
{
	ssize_t nbytes;
	unsigned char buf[BUFFSIZE];
	int request_type;
	struct client *client;

	client = clients->clients[index];

	/* If read error or FIN received */
	if ((nbytes = read(client->fd, buf, sizeof(buf) - 1)) <= 0) {
		/* If FIN received then return */
		if (nbytes == 0)
			return 0;
		else { /* Else handle error */
			perror("read error");
			return -1;
		}
	}
	buf[nbytes] = 0;

	/* Now detect request type and handle it accordingly */
	request_type = get_request_type(buf, nbytes);
	switch (request_type) {

	case REQUEST_USERN : 
		fprintf(stderr, "[*] Request type REQUEST_USERN\n");
		if (server_handle_username(clients, index, buf) == -1)
			return -1;
		break;

	case REQUEST_AVAIL : 
		fprintf(stderr, "[*] Request type REQUEST_AVAIL\n");
		server_send_available(clients, index);
		break;

	case REQUEST_ASSOC : 
		fprintf(stderr, "[*] Request type REQUEST_ASSOC\n");
		if (server_handle_assoc(clients, index, buf) == -1)
			return -1;
		break;

	case REQUEST_MESSG : 
		fprintf(stderr, "[*] Request type REQUEST_MESSG\n");
		if (server_send_message(clients, index, buf) == -1)
			return -1;
		break;

	default :
		fprintf(stderr, "[*] Request type REQUEST_UNKNW\n");
		//
		break;
	}

	return 1; /* Everything was ok */
}

int get_request_type(unsigned char *buf, ssize_t nbytes)
{
	if (nbytes < REQUEST_TEXT_LEN)
		return REQUEST_UNKNW;
	if (strncmp(buf, REQUEST_USERN_TEXT, REQUEST_TEXT_LEN) == 0)
		return REQUEST_USERN;
	if (strncmp(buf, REQUEST_AVAIL_TEXT, REQUEST_TEXT_LEN) == 0)
		return REQUEST_AVAIL;
	if (strncmp(buf, REQUEST_ASSOC_TEXT, REQUEST_TEXT_LEN) == 0)
		return REQUEST_ASSOC;
	if (strncmp(buf, REQUEST_MESSG_TEXT, REQUEST_TEXT_LEN) == 0)
		return REQUEST_MESSG;

	return REQUEST_UNKNW;
}

int server_send_available(struct clients *clients, int index)
{
	int buf_len;
	unsigned char username[CLIENT_USERNAME_MAX_LEN];
	unsigned char buf[BUFFSIZE] = {0};
	struct client *tmp;

	for (int i = 0; i < clients->clients_i; i++) {
		tmp = clients->clients[i];
		/* If tmp is NULL or current client is client him self or
		 * client is assocciated with other then skip */
		if (tmp == NULL
				|| tmp->fd == clients->clients[index]->fd
				|| tmp->is_assoc)
			continue;

		sprintf(username, "[*] <%s>\n", clients->clients[i]->nick);
		strcat(buf, username);
	}

	/* TODO What happens when no client is online then empty data 
	 * will be sent to client by write?? */
	if ((buf_len = strlen(buf)) == 0) {
		buf_len = sprintf(buf, "No available clients");
	}

	/* Send available user list to client */
	if (write(tmp->fd, buf, buf_len) == -1) {
		perror("server_send_available: write error");
		return -1;
	}

	return 0;
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

bool server_validate_username(struct clients *clients, int index)
{
	char regexstr[32];
	int len;
	regex_t regex;
	bool ret = false;
	char *username;

	username = clients->clients[index]->nick;

	/* Username validation */
	if ((len = strlen(username)) > CLIENT_USERNAME_MAX_LEN) {
		printf("[*] Invalid username: max %d characters allowed",
				CLIENT_USERNAME_MAX_LEN);
		return false;
	}
	sprintf(regexstr, "^[a-zA-Z0-9]{1,%d}$", CLIENT_USERNAME_MAX_LEN);

	if (regcomp(&regex, regexstr, REG_EXTENDED) != 0) {
		fprintf(stderr, "[*] client_validate_username: regcomp error\n");
		goto out;
	}

	if (regexec(&regex, username, 0, NULL, 0) == REG_NOMATCH) {
		printf("[*] Invalid username: Onlye a-Z, A-Z and 0-9 characters are allowed\n");
		goto out;
	}

	/* Check username is unique */
	for (int i = 0; i < clients->clients_i; i++) {
		if (i == index || clients->clients[i] == NULL)
			continue;
		if (strcmp(clients->clients[i]->nick,
					clients->clients[index]->nick) == 0) {
			goto out;
		}
	}

	ret = true;

out:
	regfree(&regex);

	return ret;
}

int server_handle_username(struct clients *clients, int index, char *buf)
{
	struct client *client;

	client = clients->clients[index];

	strncpy(client->nick, buf + REQUEST_TEXT_LEN, CLIENT_USERNAME_MAX_LEN);
	client->nick[CLIENT_USERNAME_MAX_LEN] = 0;

	if (server_validate_username(clients, index) == false) {
		fprintf(stderr, "[*] Username validation failed\n");
		client->nick[0] = 0;
		/* Tell the client */
		if (write(client->fd,
					RESPONSE_USERN_ERR, sizeof(RESPONSE_USERN_ERR) - 1) < 0) {
			perror("write error: Failed to send username check response");
			return -1;
		}

		return -1;
	}
	if (write(client->fd,
				RESPONSE_USERN_OK, sizeof(RESPONSE_USERN_OK) - 1) < 0) {
		perror("write error: Failed to send username check response");
		return -1;
	}

	return 0;
}

int	server_handle_assoc(struct clients *clients, int index, char *buf)
{
	struct client *client, *tmp;
	char username_assoc[CLIENT_USERNAME_MAX_LEN + 1];
	char res_buf[BUFFSIZE];
	int nbytes, ret, i;

	ret = 0;
	client = clients->clients[index];

	/* Copying the username from request */
	strncpy(username_assoc, buf + REQUEST_TEXT_LEN, CLIENT_USERNAME_MAX_LEN);
	username_assoc[CLIENT_USERNAME_MAX_LEN] = 0;

	/* If username_assoc is the same as client username then reject */
	if (strcmp(client->nick, username_assoc) == 0) {
		nbytes = sprintf(res_buf, "%s[!] You can not assocciate with yourself",
				RESPONSE_RESER);
		write(client->fd, res_buf, nbytes);
		return -1;
	}

	/* Find the client with username username_assoc */
	for (i = 0; i < clients->clients_i; i++) {
		tmp = clients->clients[i];
		if (tmp == NULL || client->fd == tmp->fd)
			continue;

		if (strncmp(tmp->nick, username_assoc,
					CLIENT_USERNAME_MAX_LEN) == 0) {
			nbytes = sprintf(res_buf, "%s[*] You are now assocciated with <%s>",
					RESPONSE_RESOK, username_assoc);
			/* Tell requesting client that you are assocciated */
			if ((nbytes = write(client->fd, res_buf, nbytes)) == -1) {
				perror("server_handle_assoc: write error");
				ret = -1;
				break;
			}
			nbytes = sprintf(res_buf, "%s[*] You are now assocciated with <%s>",
					RESPONSE_RESOK, client->nick);
			/* Tell the assocciated partner that he/she is assocciated
			 * with requested client */
			if ((nbytes = write(tmp->fd, res_buf, nbytes)) == -1) {
				perror("server_handle_assoc: write error");
				ret = -1;
				break;
			}
			/* Assocciate chat pair */
			client->pair_fd = tmp->fd;
			tmp->pair_fd = client->fd;
			break;
		}
	}
	if (i == clients->clients_i) {
		nbytes = sprintf(res_buf, "%s[!] No user found with username <%s>",
				RESPONSE_RESER, username_assoc);
		write(client->fd, res_buf, nbytes);
		ret = -1;
	}

	return ret;
}

int	server_send_message(struct clients *clients, int index, char *buf)
{
	struct client *client;
	char *msgptr, res_buf[BUFFSIZE];
	int nbytes;

	msgptr = buf + REQUEST_TEXT_LEN;
	client = clients->clients[index];

	/* If client at index is not assocciated then he can not send message */
	if (!server_isclient_assoc(client)) {
		nbytes = sprintf(res_buf,
				"%s[!] You are not assocciated with other user",
				RESPONSE_RESER);
		if (write(client->pair_fd, res_buf, nbytes) == -1)
			fprintf(stderr, "[!] server_send_message: write error\n");
		return -1;
	}

	/* Send the message to assocciated client */
	nbytes = sprintf(res_buf, "%s%s", RESPONSE_RESOK, msgptr);
	if (write(client->pair_fd, res_buf, nbytes) == -1) {
		fprintf(stderr, "[!] server_send_message: write error\n");
		return -1;
	}

	return 0;
}

bool server_isclient_assoc(struct client *client)
{
	return client->pair_fd != -1;
}
