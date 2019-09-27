#ifndef __CHAT_H
#define __CHAT_H

#include <stdbool.h>

#define PEER_TERMINATED 0x10

#define REQUEST_USERN 0x01 		/* Check username 				 */
#define REQUEST_AVAIL 0x02		/* List available online clients */
#define REQUEST_ASSOC 0x03		/* Assocciate client with client */
#define REQUEST_MESSG 0x04		/* Send message to client		 */
#define REQUEST_UNKNW 0x05		/* Unknown request				 */

#define REQUEST_USERN_TEXT "USERN"
#define REQUEST_AVAIL_TEXT "AVAIL"
#define REQUEST_ASSOC_TEXT "ASSOC"
#define REQUEST_MESSG_TEXT "MESSG"
#define REQUEST_CQUIT_TEXT "CQUIT"
#define REQUEST_TEXT_LEN   5

#define RESPONSE_USERN_OK  "USERNOK"
#define RESPONSE_USERN_ERR "USERNERR"

#define RESPONSE_RESOK "RESOK"
#define RESPONSE_RESER "RESER"

#define CLIENT_CMD_CODE_CHAT 0x01
#define CLIENT_CMD_CODE_MESG 0x02
#define CLIENT_CMD_CODE_DISC 0x03
#define CLIENT_CMD_CODE_QUIT 0x04
#define CLIENT_CMD_CODE_UKNW 0x09

/**
 * Commands
 * /chat <username>    start conversession with <username>
 * /disc               disconnect
 * /quit               quit the application
 *
 */
#define CLIENT_CMD_TEXT_CHAT "CHAT"
#define CLIENT_CMD_TEXT_DISC "DISC"
#define CLIENT_CMD_TEXT_QUIT "QUIT"
#define CLIENT_CMD_TEXT_LEN   4

#define CLIENT_USERNAME_MAX_LEN 8

struct client {
	int fd;
	int pair_fd;
	/* nclient = number of clients in clients array, only to
	 * check connected client limit */
	unsigned char username[CLIENT_USERNAME_MAX_LEN + 1];
	bool is_username_set;
	bool is_assoc;
};

struct clients {
	struct client *clients[FD_SETSIZE];
	size_t nclient;
	size_t clients_i;
};

//static char errors[BUFFSIZE];

int 	server_handle_request(struct clients *clients, int index);
int 	get_request_type(unsigned char *buf, ssize_t nbytes);
int		server_new_client(struct clients *clients, int sockfd);
int		server_del_client(struct clients *clients, int index);
int		server_get_request_type(int sockfd);
int 	server_send_available(struct clients *clients, int index);
bool	server_validate_username(struct clients *clients, int index);
int		server_handle_username(struct clients *clients, int index, char *buf);
int		server_handle_assoc(struct clients *clients, int index, char *buf);
int		server_send_message(struct clients *clients, int index, char *buf);
bool	server_isclient_assoc(struct client *client);

int 	client_handle_command(struct client *client);
int 	client_handle_response(struct client *client);
int 	client_username_check(int sockfd, const char *username);
int 	client_get_online_users(struct client *client, unsigned char *buf,
		size_t size);
bool	client_validate_username(const char *username, char *errors);
int		client_request_assoc(struct client *client, char *errors,
		const char *username);
int 	client_get_command_type(const char *cmd);
int		client_send_message(struct client *client, const char *msg);

#endif
