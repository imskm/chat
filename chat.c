#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>

#include "easyio.h"
#include "chat.h"

static int prepare_request_for_message(struct request *req,
		const char *cmd_bufp);
static bool chat_is_request_msg(struct request *req);
static int chat_get_request_type(int status);


int chat_command_handle(struct client *client)
{
	int 				nbytes, ret;
	String 				cmd_buf;
	struct request 		req = {0};

	ret = 0;

	/* 1. Get command from stdin and detect command type */
	if (GetString(&cmd_buf, stdin) == 0)
		return 0;

	str_trim(cmd_buf);

	/* Set the nick of user */
	req.src = client;
	if (client->is_username_set)
		req.dest = client->pair;

	/* 2. Prepare the request struct */
	if (chat_command_prepare(&req, cmd_buf) == -1)
		return -1;

	/* 3. Send the request to server */
	if (chat_request_send(client, &req) == -1) {
		ret = -1;
		goto cleanup;
	}

	/* Clean up request */
cleanup:
	request_cleanup(&req);

	return ret;
}

int	chat_command_prepare(struct request *req, const char *cmd_buf)
{
	int cmd_index;
	const struct command *cmd;
	const char *cmd_bufp;

	cmd_bufp = cmd_buf;

	/* 1. If user has not given command but typed something then it's special
	 * case and this should be treated as message command */
	if (*cmd_bufp != '/')
		return prepare_request_for_message(req, cmd_bufp);

	/* 1.1. Handle first phase of the command */
	if ((cmd_index = command_handle(&cmd_bufp)) == -1) {
		fprintf(stderr, "[!] Unknown command: %s\n", cmd_buf);
		return -1;
	}

	cmd 			= &commands[cmd_index];
	req->cmd 		= cmd->cmd;
	req->irc_cmd 	= cmd->irc_cmd;

	/* 2. Since each command has its own unique structure and number of
	 *    required parameters therefor each command should be handled
	 *    by seperate function of its own and this function should populate
	 *    the request struct accordingly */
	if (cmd->handle(req, cmd_bufp) == -1)
		return -1;

	return 0;
}

int	chat_request_send(struct client *client, struct request *req)
{
	int nbytes;
	char buf[BUFFSIZE];

	/* TODO buf must not exceed 512 bytes because its the IRC message
	 * len limit */

	nbytes = sprintf(buf, ":%s %s", req->src->nick, req->irc_cmd);

	/* Set destination (if any) */
	if (req->dest)
		nbytes = sprintf(buf, "%s %s", buf, req->dest);

	/* Set params */
	for (int i = 0; req->params[i] != NULL; i++)
		nbytes = sprintf(buf, "%s %s", buf, req->params[i]);

	/* Set message (if any) */
	if (req->body)
		nbytes = sprintf(buf, "%s :%s", buf, req->body);

	/* Add CRLF at the end */
	nbytes = sprintf(buf, "%s \r\n", buf);

	fprintf(stderr, "%s\n", buf);

	if (write(client->fd, buf, nbytes) == -1) {
		perror("chat_request_send: write error");
		return -1;
	}

	return 0;
}

static int prepare_request_for_message(struct request *req,
		const char *cmd_bufp)
{
	/* If user is not associated with other user then he/she
	 * can not send message without using /msg command */
	if (!req->dest) {
		fprintf(stderr, "[!] Can't send message, you are not associated\n");
		return -1;
	}
	req->irc_cmd = commands[command_message_get_index()].irc_cmd;
	req->body = strdup(cmd_bufp);
	return 0;
}



/* Server functions */

int chat_request_handle(struct collection *collection)
{
	char buf[BUFFSIZE];
	struct clients *clients;
	struct client *client;
	ssize_t nbytes;
	struct request req = {0};
	size_t index;

	/* Set up client(s) varible to access it easily */
	clients = collection->clients;
	index   = collection->index;
	client  = clients->clients[index];

	/* 1. Read the input and If FIN received then return */
	if ((nbytes = read(client->fd, buf, sizeof(buf) - 1)) == 0) {
		if (clients->clients[index]->nick[0])
			fprintf(stderr, "[*] Client <%s> terminated.\n",
					client->nick);
		else
			fprintf(stderr, "[*] Client [%d] terminated.\n",
					client->fd);

		return chat_client_session_close(clients, index);
	} else if (nbytes == -1) { /* Else handle error */
		perror("read error");
		return -1;
	}
	buf[nbytes] = 0;

	/* If client doesn't have a nick then don't allow to perform any action */
	fprintf(stderr, "nick %s\n", client->nick);
	if (!client->nick[0]) {
		sprintf(buf, "%d :You don't have a nick yet.", RPL_NONE);
		response_send(client->fd, buf, strlen(buf));
		return -1;
	}

	str_trim(buf);
	/* Set the source of request (client struct pointer) */
	req.src = client;
	collection->buf = buf;

	/* 2. Prepare the request struct. If error occurs then handle the error
	 *    with error response function group */
	if (chat_request_prepare(&req, collection) == -1) {
		/* Send error reply to request sender */
		int err_i = chat_calc_reply_index(req.status);
		responses[err_i].handle(&req, collection);
		return -1;
	}


	/* 3. Response send */
	/* If request is for sending message then call message sending function*/
	if (chat_get_request_type(req.status) == REQTYPE_RPL) {
		int index = chat_calc_reply_index(req.status);
		return responses[index].handle(&req, collection);
	}

	/* Send message */
	response_send_msg(&req, collection);

	return 0;
}

int chat_request_prepare(struct request *req, struct collection *collection)
{
	int cmd_index;
	const struct command *cmd;
	const char *cmd_bufp;

	cmd_bufp = collection->buf;

	/* 1.1. Handle first phase of the request command message */
	if ((cmd_index = request_handle(req, collection)) == -1) {
		req->status = ERR_UNKNOWNCOMMAND; /* only error request_handle check */
		fprintf(stderr, "[!] Unknown command: %s\n", cmd_bufp);
		return -1;
	}

	/* If request command is PRIVMSG and server don't know the nick of sender
	 * then he can't send message to other. Set proper request status and
	 * return */
	if (!chat_is_request_msg(req)) {
		//
		return 0;
	}

	cmd 			= &commands[cmd_index];
	req->cmd 		= cmd->cmd;
	req->irc_cmd 	= cmd->irc_cmd;

	/* 2. Since each request has its own unique structure and number of
	 *    required parameters therefor each request should be handled
	 *    by seperate function of its own and this function should populate
	 *    the request struct accordingly */
	if (cmd->request(req, collection) == -1)
		return -1;

	return 0;
}

int chat_client_session_close(struct clients *clients, int index)
{
	if (clients->clients[index] == NULL)
		return -1;

	free(clients->clients[index]);
	clients->clients[index] = NULL;

	return 0;
}

bool chat_is_request_msg(struct request *req)
{

	return true;
}

bool chat_validate_nick(const char *nick)
{
	char regexstr[32];
	int len;
	regex_t regex;
	bool ret = false;

	/* nick validation */
	if ((len = strlen(nick)) > CLIENT_USERNAME_MAX_LEN) {
		fprintf(stderr, "[*] Invalid username: max %d characters allowed\n",
				CLIENT_USERNAME_MAX_LEN);
		return false;
	}
	sprintf(regexstr, "^[a-zA-Z0-9]{1,%d}$", CLIENT_USERNAME_MAX_LEN);

	if (regcomp(&regex, regexstr, REG_EXTENDED) != 0) {
		fprintf(stderr, "[*] client_validate_username: regcomp error\n");
		goto out;
	}

	if (regexec(&regex, nick, 0, NULL, 0) == REG_NOMATCH) {
		fprintf(stderr, "[*] Invalid username: Onlye a-Z, A-Z and 0-9 "
				"characters are allowed\n");
		goto out;
	}

	ret = true;

out:
	regfree(&regex);

	return ret;
}

int chat_find_nick(struct clients *clients, const char *nick)
{
	for (int i = 0; i < clients->clients_i; i++)
		if (strcmp(clients->clients[i]->nick, nick) == 0)
			return i;

	return -1;
}

int chat_calc_reply_index(int status)
{
	if (status < IRC_MIN_REPLY_CODE || status > IRC_MAX_REPLY_CODE)
		return -1;

	return status - IRC_MIN_REPLY_CODE;
}

static int chat_get_request_type(int status)
{
	if (status >= 400)
		return REQTYPE_ERR;

	if (status >= 200 && status <= 399)
		return REQTYPE_RPL;

	return REQTYPE_MSG;
}

int chat_response_handle(struct client *client)
{
	unsigned char buf[BUFFSIZE];
	int nbytes;

	if ((nbytes = read(client->fd, buf, sizeof(buf) - 1)) == 0) {
		return PEER_TERMINATED;
	} else if (nbytes == -1) {
		perror("[!] chat_response_handle: read error");
		return -1;
	}
	buf[nbytes] = 0;

	puts(buf);

	return 0;
}
