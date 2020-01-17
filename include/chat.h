#ifndef __CHAT_H
#define __CHAT_H

#include <stdbool.h>
#include <libsocket.h>
#include <str.h>


#define PEER_TERMINATED 0x10

#define CLIENT_QUIT   -001

#define CLIENT_USERNAME_MAX_LEN         9
#define COMMAND_MSG_BUF_MAX_LEN         512
#define MAX_CHANNEL_CONNECTION_ALLOWED  5

#define IRC_MIN_REPLY_CODE 200
#define IRC_MAX_REPLY_CODE 502

#define REQTYPE_MSG 1
#define REQTYPE_ERR 2
#define REQTYPE_RPL 2

struct client {
	int fd;
	int pair_fd;
	struct client *partner;
	unsigned char nick[CLIENT_USERNAME_MAX_LEN + 1];
	bool is_username_set;
	bool is_assoc;
};

struct clients {
	struct client *clients[FD_SETSIZE];
	/* nclient = number of clients in clients array, only to
	 * check connected client limit */
	size_t nclient;
	size_t clients_i;
};

/* This struct will be used to pass all data to different functions
 * as the number of arguments is large and some function requires all
 * of these data to perform required action */
struct collection {
	struct clients *clients;
	struct channels *channels;
	size_t index;
	const char *buf;
};

#include "request.h"
#include "command.h"
#include "response.h"
#include "channel.h"

int		server_new_client(struct clients *clients, int sockfd);
int		server_del_client(struct clients *clients, int index);

int 	client_nick_update(const char *nick);
int     client_channelname_update(void);
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
void    client_temp_channelname_set(const char *temp_channelname);
void	client_quit_set();

/**
 *  From the client side sequence of function call is as:
 *   1. chat_command_handle() is called from main
 *   2. chat_command_prepare() is called from chat_command_handle()
 *      i.  chat_command_parse() is called
 *      ii. chat_command_prepare_CMD() is called (CMD is specific command)
 *   3. chat_reuqest_send() is called from chat_command_handle()
 */
int		chat_command_handle(struct client *client, char *cmd_buf);
int		chat_command_prepare(struct request *req, const char *cmd_buf);
int		chat_request_send(struct client *client, struct request *req);
int 	chat_response_handle(struct client *client);

int 	chat_request_handle(struct collection *collection, fd_set *set);
int 	chat_request_prepare(struct request *req,
		struct collection *collection);
int 	chat_response_prepare(struct request *req, struct clients *clients,
		size_t index);

int 	chat_client_session_open(struct clients *clients,
		struct client *client);
int 	chat_client_session_close(struct clients *clients, int index);

/* Including Errors Code will be used for Error replies */
#include "chat_errors.h"
#include "chat_replies.h"

bool 	chat_validate_nick(const char *nick);
bool    chat_validate_channelname(const char *channelname);
int		chat_find_nick(struct clients *clients, const char *nick);
int		chat_find_channelname(struct channels *channels, const char *channelname);
char	*chat_serialize_nick(struct client **clients, int nclients, char *buf, size_t size);

int 	chat_render_line(struct request *req, const char *buf, unsigned char *line);
int 	chat_calc_reply_index(int status);
int		chat_message_parse(unsigned char *msg, struct request *req);
bool	isinteger(unsigned char *str);

void 	chat_print_line(const char *line);
void 	chat_info_printline(const char *line);
char 	*chat_construct_info_line(const char *info, char *out_line);

#endif
