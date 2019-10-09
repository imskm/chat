#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int request_dest_set(struct request *req, const char *dest)
{
	if (dest == NULL)
		return -1;
	req->dest = strdup(dest);

	return 0;
}

int request_cleanup(struct request *req)
{
	if (req->dest) free((void *)req->dest);
	/* req->nick should not be freed */
	for (int i = 0; req->params[i] && i < REQUEST_MAX_PARAMS; i++) {
		free(req->params[i]);
		req->params[i] = NULL;
	}
	if (req->body) free(req->body);

	req->dest = NULL;
	req->body = NULL;

	return 0;
}


int request_handle(struct request *req, struct collection *collection)
{
	char *bufp = strdup(collection->buf);
	char *parts[3];
	int n, ret;

	ret = -1;

	/* Find second param from the buf */
	if((n = str_split(bufp, " ", parts, 3)) != 2) 
		goto cleanup;

	str_trim(parts[0]), str_trim(parts[1]);

	/* Check for empty command
	 * parts[0] = :nick,  parts[1] = COMMAND */
	if (strlen(parts[0]) <= 2 || strlen(parts[1]) <= 1)
		goto cleanup;

	for (int i = 0; commands[i].cmd; i++) {
		if (strcmp(parts[1], commands[i].irc_cmd) == 0) {
			ret = i;
			goto cleanup;
		}
	}

cleanup:
	free(bufp);

	return ret;
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
	char *parts[4] = {0}, *p;

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

	request_dest_set(req, parts[2]);
	req->status = RPL_NONE;
	req->body = strdup("Nick has been changed successfully!");
	/* TODO Notify to other users about nick change */

cleanup:
	free(p);

	return ret;
}

int request_handle_quit(struct request *req, struct collection *collection)
{
	puts("request_handle_quit");

	return 0;
}

