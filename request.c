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
	puts("request_handle_msg");

	return 0;
}

int request_handle_names(struct request *req, struct collection *collection)
{
	puts("request_handle_names");

	return 0;
}

int request_handle_nick(struct request *req, struct collection *collection)
{
	puts("request_handle_nick");

	return 0;
}

int request_handle_quit(struct request *req, struct collection *collection)
{
	puts("request_handle_quit");

	return 0;
}

