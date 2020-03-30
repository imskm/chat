#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include <cursor.h>

#include "chat.h"

static int prepare_request_for_message(struct request *req,
		const char *cmd_bufp);
static bool chat_is_request_msg(struct request *req);
static int chat_get_request_type(int status);

int chat_command_handle(struct client *client, char *cmd_buf)
{
	int 				ret;
	struct request 		req = {0};

	ret = 0;

	str_trim(cmd_buf);

	/* Set the nick of user */
	req.src = client;

	/* 2. Prepare the request struct */
	if (chat_command_prepare(&req, cmd_buf) == -1)
		return -1;

#ifdef CLIENT_APP
	/* If command is quit then set quit flag */
	if (req.status == CLIENT_QUIT)
		client_quit_set();
#endif

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
		char tmp[256];
		sprintf(tmp, "Unknown command: %s", cmd_buf);
		chat_info_printline(tmp);
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
	char buf[BUFFSIZE], tmp[BUFFSIZE];

	/* TODO buf must not exceed 512 bytes because its the IRC message
	 * len limit */

	sprintf(buf, ":%s %s", req->src->nick, req->irc_cmd);

	/* Set params */
	for (int i = 0; req->params[i] != NULL; i++) {
		sprintf(tmp, " %s", req->params[i]);
		strcat(buf, tmp);
	}

	/* Set message (if any) */
	if (req->body) {
		sprintf(tmp, " :%s", req->body);
		strcat(buf, tmp);
	}

	/* Add CRLF at the end */
	strcat(buf, "\r\n");

	//fprintf(stderr, "%s\n", buf);

	if (write(client->fd, buf, strlen(buf)) == -1) {
		chat_info_printline("chat_request_send: write error");
		return -1;
	}

	return 0;
}

// Todo : channel message handling
static int prepare_request_for_message(struct request *req,
		const char *cmd_bufp)
{
	 /* message handling if user tries to send message 
	  * without using '/msg' command */
	
	char *target_channel;
	char *tmp = strdup(cmd_bufp);

	/* If user is not associated with other user then he/she
	 * can not send message without using /msg command */
	/* check for associated channel. I am no longer allowing user to
	 * associate with other user, instead allowing user to associate with
	 * channel */

	#ifdef CLIENT_APP

	target_channel = client_active_channel();

	#endif

	if (target_channel == NULL)
		return -1;

	req->irc_cmd = commands[command_message_get_index()].irc_cmd;
	req->cmd = commands[command_message_get_index()].cmd;
	request_param_set(req, target_channel);
	request_body_set(req, tmp);
	// request_dump(req);

	free(tmp);

	return 0;
}



/* Server functions */

int chat_request_handle(struct collection *collection, fd_set *set)
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
			fprintf(stderr, "[*] Client <%s> terminated.\n", client->nick);
		else
			fprintf(stderr, "[*] Client [%d] terminated.\n", client->fd);

		/* Clear fd from read set */
		FD_CLR(client->fd, set);

		return chat_client_session_close(clients, index);
	} else if (nbytes == -1) { /* Else handle error */
		perror("read error");
		return -1;
	}
	buf[nbytes] = 0;
	fprintf(stderr, "[*] Client Request: %s\n", buf);

	/* If client doesn't have a nick then don't allow to perform any action */
	/*
	if (!client->nick[0]) {
		sprintf(buf, "%d :You don't have a nick yet.", RPL_NONE);
		response_send(client->fd, buf, strlen(buf));
		return -1;
	}
	*/

	str_trim(buf);
	/* Set the source of request (client struct pointer) */
	req.src = client;
	collection->buf = buf;
	/* 1.1 Parse the request */
	if (request_parse(&req, buf) == -1) {
		fprintf(stderr, "parser error: failed to parse request '%s'\n",
				buf);
		return -1;
	}
	/* Leave it here for DEBUGGING
	request_dump(&req);
	*/

	/* @Note: Developer's note: all the error checking must be done in
	 *        request_handle_**() handler function and handler must set
	 *        proper error code in req struct in case of error and must
	 *        return -1. For example see request_handle_nick() function.
	 *        response handler response_handle_**() must not perform error
	 *        check because all the error check is done in request handler.
	 *        Response handler always gets correct req struct and it must
	 *        only focus on handling response and not validation check etc.
	 *
	 * Error: If you want to handle the error by your self then register your
	 *        error handler in responses struct array in rescodes.h file.
	 *        Otherwise you just leave that member as NULL, and default error
	 *        handler will take care of that. For this default handler to work
	 *        properly you must set correct error code in status member of
	 *        request struct. */

	/* 2. Prepare the request struct. If error occurs then handle the error
	 *    with error response function group */
	if (chat_request_prepare(&req, collection) == -1) {
		request_dump(&req);
		/* Send error reply to request sender */
		int err_i = chat_calc_reply_index(req.status);
		if (responses[err_i].handle != NULL) /* Error handler is called */
			responses[err_i].handle(&req, collection); 
		else
			response_send_err(&req, collection);
		return -1;
	}

	request_dump(&req);

	/* If request is for nick then handle it here because RPL_WELCOME
	 * does not exist in response array */
	if (req.status == RPL_WELCOME) {
		return response_send_rpl_welcome(&req, collection);
	}
	
	/* If request is for quit then handle it here */
	if (req.status == CLIENT_QUIT) {
		fprintf(stderr, "[*] Quit request received\n");
		FD_CLR(client->fd, set); /* Clear fd from read set */
		close(client->fd);
		if (req.src->partner == NULL) {
			return chat_client_session_close(clients, index);
		}
		response_send_rpl_quit(&req, collection);
		return chat_client_session_close(clients, index);
	}

	/* If request is for part then handle it here */
	if (req.status == PART_LEAVE) {
		return response_send_rpl_part(&req, collection);
	}

	/* 3. Response send */
	/* If request is for sending reply to request sender */
	if (chat_get_request_type(req.status) == REQTYPE_RPL) {
		int i = chat_calc_reply_index(req.status);
		return responses[i].handle(&req, collection);
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

	/* 1.1 Parse the request */
	if (request_parse(req, cmd_bufp) == -1) {
		fprintf(stderr, "parser error: failed to parse request '%s'\n",
				cmd_bufp);
		return -1;
	}

	/* Detect the command */
	if ((cmd_index = request_handle(req, collection)) == -1) {
		req->status = ERR_UNKNOWNCOMMAND; /* only error request_handle check */
		fprintf(stderr, "Error: Unknown command '%s'\n", cmd_bufp);
		return -1;
	}

	/* If request command is PRIVMSG and server don't know the nick of sender
	 * then he can't send message to other. Set proper request status and
	 * return */
	if (!chat_is_request_msg(req)) {
		//
		return 0;
	}

	/* request_parse strdups it, therefore free is needed */
	free((void *) req->irc_cmd);
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
		char tmp[256];
		sprintf(tmp, "Invalid username: max %d characters allowed",
				CLIENT_USERNAME_MAX_LEN);
		chat_info_printline(tmp);
		return false;
	}
	sprintf(regexstr, "^[a-zA-Z0-9]{1,%d}$", CLIENT_USERNAME_MAX_LEN);

	if (regcomp(&regex, regexstr, REG_EXTENDED) != 0) {
		chat_info_printline("client_validate_username: regcomp error");
		goto out;
	}

	if (regexec(&regex, nick, 0, NULL, 0) == REG_NOMATCH) {
		chat_info_printline("Invalid username: Onlye a-Z, A-Z and 0-9 "
				"characters are allowed");
		goto out;
	}

	ret = true;

out:
	regfree(&regex);

	return ret;
}


bool chat_validate_channelname(const char *channelname)
{
	char regexstr[32];
	int len;
	regex_t regex = {0};
	bool ret = false;
	char tmp[256];

	/* Channel validation */
	if ((len = strlen(channelname)) > CHANNEL_NAME_MAX_LEN) {
		sprintf(tmp, "Invalid Channel Name : max %d characters allowed", CHANNEL_NAME_MAX_LEN);
		chat_info_printline(tmp);
		goto out;
	}

	sprintf(regexstr, "^#[a-zA-Z0-9]{1,%d}$", CHANNEL_NAME_MAX_LEN);

	if (regcomp(&regex, regexstr, REG_EXTENDED) != 0) {
		chat_info_printline("client_validate_username: regcomp error");
		goto out;
	}

	if (regexec(&regex, channelname, 0, NULL, 0) == REG_NOMATCH) {
		chat_info_printline("Invalid Channel Name: Onlye a-Z, A-Z and 0-9 "
				"characters are allowed");
		goto out;
	}

	ret = true;

out:
	regfree(&regex);

	return ret;
}

int chat_find_nick(struct clients *clients, const char *nick)
{
	if (clients == NULL || nick == NULL)
		return -1;
	
	for (int i = 0; i < clients->clients_i; i++)
		if (clients->clients[i] && strcmp(clients->clients[i]->nick, nick) == 0)
			return i;

	return -1;
}

int chat_find_channelname(struct channels *channels, const char *channelname)
{
	if (channelname == NULL || channels == NULL)
		return -1;
	
	for (int i = 0; i < channels->nchannels; i++)
		if (channels->channels[i] && strcmp(channels->channels[i]->channelname, channelname) == 0)
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
	unsigned char buf[BUFFSIZE], line[BUFFSIZE];
	ssize_t nbytes;
	struct request req = {0};

	/* 1. Read the socket and If FIN received then return */
	if ((nbytes = read(client->fd, buf, sizeof(buf) - 1)) == 0) {
		return PEER_TERMINATED;
	} else if (nbytes == -1) { /* Else handle error */
		chat_info_printline("read error");
		return -1;
	}
	buf[nbytes] = 0;

	str_trim(buf);

	/* 2. Prepare the request structure. If response is numeric reply then
	 *    handle it here */
	/* Parsing server response message */
	if (chat_message_parse(buf, &req) == -1) {
		char tmp[256];
		sprintf(tmp, "parser error: %s", buf);
		chat_info_printline(tmp);
		return -1;
	}

	/*
	fprintf(stderr, "irc_cmd: %s\n", req.irc_cmd);
	for (int i = 0; req.params[i]; i++)
		fprintf(stderr, "param[%d]: %s\n", i + 1, req.params[i]);
	fprintf(stderr, "body: %s\n", req.body);
	fprintf(stderr, "status: %d\n", req.status);
	fprintf(stderr, "Original: %s\n", buf);
	*/

#ifdef CLIENT_APP
	/* Handle special response which requires client state change for example
	 * setting a nick will create RPL_WELCOME response from server and client's
	 * nick need to be changed in client instance and the same will be reflected
	 * in prompt */
	if (req.status == RPL_WELCOME) {
		client_nick_update(req.params[0]);
	}
	// Todo : handle response for channel join command
	if (req.status == RPL_TOPIC || req.status == RPL_NOTOPIC) {
		client_channelname_update();
	}
	
#endif

	chat_render_line(&req, req.body, line);
	chat_print_line(line);

	return 0;
}

char *chat_serialize_nick(struct client **clients, int nclients, char *buf, size_t size)
{
	char *p;
	buf[0] = 0;

	p = buf;
	for (int i = 0; i < nclients; i++) {
		if (clients[i] == NULL)
			continue;
		int j = 0;
		while (*p = clients[i]->nick[j++]) p++;

		if (size - (p - buf) < CLIENT_USERNAME_MAX_LEN + 1) {
			*p = 0;
			return buf;
		}
		*p = ' ';
		p++;
	}
	*p = 0;

	return buf;
}

int chat_message_parse(unsigned char *msg, struct request *req)
{
	unsigned char parts[512], *p, *prevp;

	prevp = p = msg;
	if (*prevp != ':')
		return -1;

	/* 1. Extract <origin> */
	if ((p = strchr(prevp, ' ')) == NULL)
		return -1;

	strncpy(parts, ++prevp, p - prevp);
	parts[p - prevp] = 0;
	request_orig_set(req, parts);

	prevp = p++;
	while (isspace(*prevp)) prevp++; /* Skip white spaces */

	/* 2. Extract Command */
	if ((p = strchr(prevp, ' ')) == NULL)
		return -1;

	strncpy(parts, prevp, p - prevp);
	parts[p - prevp] = 0;
	if (isinteger(parts)) {
		req->status = atoi(parts);
		req->status = req->status >= 0 ? req->status : -req->status;
	} else {
		req->irc_cmd = strdup(parts);
	}
	prevp = p++;
	while (isspace(*prevp)) prevp++; /* Skip white spaces */

	/* IF reached at the end of the message then exit */
	if (*prevp == '\r' && *(prevp + 1) == '\n')
		return 0;

	/* 3. Extract param -> a: first param is nick (receiver) */
	if ((p = strchr(prevp, ' ')) == NULL)
		return -1;
	strncpy(parts, prevp, p - prevp);
	parts[p - prevp] = 0;
	request_param_set(req, parts);
	prevp = p++;
	while (isspace(*prevp)) prevp++; /* Skip white spaces */

	/* IF reached at the end of the message then exit */
	if (*prevp == '\r' && *(prevp + 1) == '\n')
		return 0;

	/* 4. Extract next param -> b: can be client's message or server's
	 *    reply message */
	while (*prevp) {
		if (*prevp == ':') {
			request_body_set(req, prevp + 1);
			return 0;
		}

		/* Since next param is not message prefixed with ':' extract next
		 * param */
		if ((p = strchr(prevp, ' ')) == NULL) {
			request_param_set(req, prevp);
			return -1;
		}
		strncpy(parts, prevp, p - prevp);
		parts[p - prevp] = 0;
		request_param_set(req, parts);
		prevp = p++;
		while (isspace(*prevp)) prevp++; /* Skip white spaces */
	}

	return 0;
}

bool isinteger(unsigned char *str)
{
	str_trim(str);
	if (str[0] == '-' || str[1] == '+')
		str++;

	for (int i = 0; str[i]; i++) {
		if (!isdigit(str[i]))
			return false;
	}

	return true;
}

int chat_render_line(struct request *req, const char *buf, unsigned char *line)
{
	unsigned char part[128];
	struct tm *tm;
	time_t t;

	if ((t = time(NULL)) == ((time_t) -1))
		return -1;
	tm = localtime(&t);
	sprintf(line, "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
	if (req->status == 0) {
		sprintf(part, "\033[1m%13s\033[0m \033[32m|\033[0m ", req->orig);
		strcat(line, part);
	} else {
		sprintf(part, "\033[1m%13s\033[0m \033[32m|\033[0m ", " ");
		strcat(line, part);
	}

	if (req->status >= 400) 
		strcat(line, "\033[31m");
	else if (req->status >= 300) 
		strcat(line, "\033[33m");

	if (req->status >= 300 && req->body) {
		strcat(line, req->body);
		strcat(line, "\033[0m");
	} else if (req->body) {
		strcat(line, req->body);
	}

	return 0;
}

void chat_info_printline(const char *line)
{
	char log_msgline[256];

	chat_construct_info_line(line, log_msgline);
	chat_print_line(log_msgline);
}

void chat_print_line(const char *line)
{
	cur_up(1);
	cur_toleft();
	fprintf(stdout, "\033[0K"); /* Clear line */
	cur_toleft();
	fprintf(stdout, "%s\n\n", line);
}

char *chat_construct_info_line(const char *info, char *out_line)
{
	struct tm *tm;
	time_t t;

	if ((t = time(NULL)) == ((time_t) -1))
		return NULL;
	tm = localtime(&t);
	sprintf(out_line,
			"%02d:%02d:%02d\033[1m\033[31m%13s\033[0m \033[32m| "
			"\033[31m%s\033[0m", tm->tm_hour, tm->tm_min, tm->tm_sec,
			"[*]", info);

	return out_line;
}

