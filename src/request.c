#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "chat.h"

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

int request_dest_set(struct request *req, const char *dest)
{
	if (dest == NULL)
		return -1;
	req->dest = strdup(dest);

	return 0;
}

int request_cleanup(struct request *req)
{
	if (req->dest) 		free((void *)req->dest);
	if (req->orig) 		free((void *)req->orig);
	/* req->nick should not be freed */
	for (int i = 0; req->params[i] && i < REQUEST_MAX_PARAMS; i++) {
		free(req->params[i]);
		req->params[i] = NULL;
	}
	if (req->body) free(req->body);

	req->dest = NULL;
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
	int n, ret = 0;
	char *parts[4] = {0}, *p;

	p = strdup(collection->buf);

	if ((n = str_split(p, " ", parts, 4)) != 3) {
		req->status = ERR_NEEDMOREPARAMS;
		goto cleanup;
	}
	request_dest_set(req, parts[2]);

	/* TODO this function must only deal with channel joining and not user
	 * joining with other user. But for now I am implementing user chatting */

	/* If argument for JOIN command is not given then return error */
	if (chat_validate_nick(parts[2]) == false) {
		req->status = ERR_NOSUCHNICK;
		goto cleanup;
	}

	request_dest_set(req, parts[2]);
	req->status = RPL_TOPIC;

cleanup:
	free(p);

	return ret;
}

int request_handle_msg(struct request *req, struct collection *collection)
{
	int ret;
	char *parts[4], *p, *q;

	p = strdup(collection->buf);

	ret = -1;
	if (str_split(p, " ", parts, 4) != 3) {
		req->status = ERR_NORECIPIENT;
		goto cleanup;
	}

	/* If empty message is given the return error and set status */
	if ((q = strchr(collection->buf + 1, ':')) == NULL) {
		req->status = ERR_NEEDMOREPARAMS; /* Should be Message body empty */
		goto cleanup;
	}
	ret = 0;
	req->status = 0; /* Message status code */
	request_dest_set(req, parts[2]);
	request_body_set(req, q + 1);

cleanup:
	free(p);

	return ret;
}

int request_handle_names(struct request *req, struct collection *collection)
{
	/* request for NAMES doesn't need any operation here */
	req->status = RPL_NAMREPLY;

	return 0;
}

int request_handle_nick(struct request *req, struct collection *collection)
{
	int n, ret = 0;
	char *parts[4] = {0}, *p, buf[BUFFSIZE];

	p = strdup(collection->buf);

	/* If argument for JOIN command is not given then return error */
	if ((n = str_split(p, " ", parts, 4)) != 3) {
		req->status = ERR_NONICKNAMEGIVEN;
		ret = -1;
		goto cleanup;
	}
	request_dest_set(req, parts[2]);

	/* If given nick fails in validation then return error */
	if (chat_validate_nick(parts[2]) == false) {
		req->status = ERR_ERRONEUSNICKNAME;
		ret = -1;
		goto cleanup;
	}

	/* If provided new nick exist then return error */
	if (chat_find_nick(collection->clients, parts[2]) != -1) {
		req->status = ERR_NICKNAMEINUSE;
		ret = -1;
		goto cleanup;
	}

	/* Since all the check passed therefore nick is assigned to this client */
	strcpy(collection->clients->clients[collection->index]->nick, parts[2]);
	sprintf(buf, "Welcome to the Internet Relay Network %s", req->src->nick);

	request_dest_set(req, parts[2]);
	request_body_set(req, buf);
	req->status = RPL_WELCOME;
	/* TODO Notify to other users about nick change */

cleanup:
	free(p);

	return ret;
}

int request_handle_quit(struct request *req, struct collection *collection)
{
	puts("request_handle_quit");
	req->status = CLIENT_QUIT;
	if (req->body) {
		request_body_set(req, req->body);
	}

	return 0;
}

