#include <stdio.h>
#include <libsocket.h>

#include "easyio.h"
#include "chat.h"

static int prepare_request_for_message(struct request *req,
		const char *cmd_bufp);


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
	req.src = client->nick;
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

	nbytes = sprintf(buf, ":%s %s", req->src, req->irc_cmd);

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

int chat_request_handle(struct clients *clients, int index)
{
}
