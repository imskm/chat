#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "chat.h"

void request_dump(struct request *req)
{
	fprintf(stderr, "Origin : %s\n", req->orig);
	fprintf(stderr, "Command: %s\n", req->irc_cmd);
	fprintf(stderr, "Params :\n");

	for (int i = 0; req->params[i] != NULL; i++)
		fprintf(stderr, "      %d: %s\n", i + 1, req->params[i]);

	if (req->body)
		fprintf(stderr, "Body   : %s\n", req->body);
}

int request_param_set(struct request *req, char *param)
{
	for (int i = 0; i < REQUEST_MAX_PARAMS; i++) {
		if (req->params[i] == NULL) {
			req->params[i]     = strdup(param);
			req->params[i + 1] = NULL;
			return 0;
		}
	}

	return -1;
}

int request_param_get(struct request *req, int index)
{

	return 0;
}

int request_body_set(struct request *req, char *body)
{
	if (body == NULL)
		return -1;
	req->body = strdup(body);

	return 0;
}

int request_orig_set(struct request *req, const char *orig)
{
	if (orig == NULL)
		return -1;
	req->orig = strdup(orig);

	return 0;
}

int request_cleanup(struct request *req)
{
	if (req->orig) 		free((void *)req->orig);
	/* req->nick should not be freed */
	for (int i = 0; req->params[i] && i < REQUEST_MAX_PARAMS; i++) {
		free(req->params[i]);
		req->params[i] = NULL;
	}
	if (req->body) free(req->body);

	req->orig = NULL;
	req->body = NULL;

	return 0;
}

int request_parse(struct request *req, const char *msg)
{
	int i, ret;
	char *prevp, *p;
	char *msgp;

	ret = -1;

	if ((msgp = strdup(msg)) == NULL) {
		fprintf(stderr, "Error: request_parese: strdup failed\n");
		return -1;
	}
	prevp = msgp;

	if (*prevp != ':') {
		fprintf(stderr, "Error: Invalid msg '%s'\n", msgp);
		goto cleanup;
	}

	prevp++;
	while (isspace(*prevp)) prevp++; /* Skip white spaces */

	/* Extract <origin> */
	if ((p = strchr(prevp, ' ')) == NULL) {
		fprintf(stderr, "Error: Invalid msg '%s'\n", msg);
		goto cleanup;
	}
	*p = 0; /* null terminate origin substring in msgp string */
	request_orig_set(req, prevp);
	*p = ' '; /* Undo null termination */
	prevp = p;

	prevp++;
	while (isspace(*prevp)) prevp++; /* Skip white spaces */

	/* Extract IRC command */
	if ((p = strchr(prevp, ' ')) == NULL) {
		req->irc_cmd = strdup(prevp);
		ret = 0; /* May be this command does not require any param */
		goto cleanup;
	}
	*p = 0; /* null terminate */
	req->irc_cmd = strdup(prevp);
	*p = ' '; /* Undo null termination */
	prevp = p;

	prevp++;
	while (isspace(*prevp)) prevp++; /* Skip white spaces */

	/* If current param is body then set it and return */
	if (*prevp == ':') {
		request_body_set(req, ++prevp);
		ret = 0;
		goto cleanup;
	}

	/* Extract all the parameters */
	for (i = 0; (p = strchr(prevp, ' ')) != NULL && i < REQUEST_MAX_PARAMS;
			i++) {
		*p = 0; /* null terminate */
		request_param_set(req, prevp);
		*p = ' '; /* Undo null termination */
		prevp = p;

		prevp++;
		while (isspace(*prevp)) prevp++; /* Skip white spaces */

		if (*prevp == ':')
			break;
	}

	if (p != NULL && i == REQUEST_MAX_PARAMS) {
		fprintf(stderr, "Error: request_parse: too many arguments\n");
		goto cleanup;
	}

	/* If current param is body then store it as body */
	if (*prevp == ':') {
		request_body_set(req, ++prevp);
	} else { /* Else it's another param set it */
		request_param_set(req, prevp);
	}
	ret = 0;


cleanup:
	free(msgp);

	return ret;
}

int request_handle(struct request *req, struct collection *collection)
{
	for (int i = 0; commands[i].cmd; i++)
		if (strcmp(req->irc_cmd, commands[i].irc_cmd) == 0)
			return i;

	return -1;
}

int request_handle_join(struct request *req, struct collection *collection)
{
	int index;
	// char buf[BUFFSIZE];

	if (chat_validate_channelname(req->params[0]) == false) {
		req->status = ERR_NOSUCHCHANNEL;
		return -1;
	}

	index = chat_find_channelname(collection->channels, req->params[0]);

	if (index != -1) {
		if(collection->channels->channels[index]->total_connected_users == CHANNEL_USERS_MAX_LEN) {
			req->status = ERR_CHANNELISFULL;
			return -1;
		}
		// requset is for joining channel, so add new user to channel
		collection->channels->channels[index]->connected_users[collection->channels->channels[index]->total_connected_users++] = req->src;
		req->status = RPL_TOPIC;

	} else if (collection->channels->nchannels == CHANNEL_MAX_LEN) {

		req->status = ERR_TOOMANYCHANNELS;
		return -1;
	} else {
		// requset is for channel creation, so create channel
		channel_create(req, collection);
		req->status = RPL_NOTOPIC;
	}


	return 0;
}

int request_handle_nick(struct request *req, struct collection *collection)
{
	char buf[BUFFSIZE];

	/* If argument for JOIN command is not given then return error */
	if (req->params[0] == NULL) {
		req->status = ERR_NONICKNAMEGIVEN;
		return -1;
	}

	/* If given nick fails in validation then return error */
	if (chat_validate_nick(req->params[0]) == false) {
		req->status = ERR_ERRONEUSNICKNAME;
		return -1;
	}

	/* If provided new nick exist then return error */
	if (chat_find_nick(collection->clients, req->params[0]) != -1) {
		req->status = ERR_NICKNAMEINUSE;
		return -1;
	}

	/* Since all the check passed therefore nick is assigned to this client */
	strcpy(collection->clients->clients[collection->index]->nick,
		req->params[0]);
	sprintf(buf, "Welcome to the Internet Relay Network %s", req->src->nick);

	request_body_set(req, buf);
	req->status = RPL_WELCOME;
	/* TODO Notify to other users about nick change */

	return 0;
}


int request_handle_msg(struct request *req, struct collection *collection)
{
	/* If recipient is missing then set proper error and return */
	if (req->params[0] == NULL) {
		req->status = ERR_NORECIPIENT;
		return -1;
	}

	/* If given nick fails in validation then return error */
	if (chat_validate_nick(req->params[0]) == false) {
		req->status = ERR_ERRONEUSNICKNAME;
		return -1;
	}

	/* If provided new nick does not exist then return error */
	if (chat_find_nick(collection->clients, req->params[0]) == -1) {
		req->status = ERR_NOSUCHNICK;
		return -1;
	}

	/* If empty message is given the return error and set status */
	if (req->body == NULL) {
		req->status = ERR_NEEDMOREPARAMS; /* Should be Message body empty */
		return -1;
	}
	req->status = 0; /* Message status code */

	return 0;
}

int request_handle_names(struct request *req, struct collection *collection)
{
	/* request for NAMES doesn't need any operation here */
	req->status = RPL_NAMREPLY;

	return 0;
}

int request_handle_quit(struct request *req, struct collection *collection)
{
	req->status = CLIENT_QUIT;

	return 0;
}

