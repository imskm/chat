#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "request.h"

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

int request_dest_set(struct request *req, char *dest)
{
	if (dest == NULL)
		return -1;
	req->dest = strdup(dest);

	return 0;
}

int request_cleanup(struct request *req)
{
	if (req->dest) free((void *)req->dest);
	for (int i = 0; req->params[i] && i < REQUEST_MAX_PARAMS; i++) {
		free(req->params[i]);
		req->params[i] = NULL;
	}
	if (req->body) free(req->body);

	req->dest = NULL;
	req->body = NULL;

	return 0;
}

