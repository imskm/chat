#include <stdio.h>

#include "request.h"

int request_param_set(struct request *req, char *param)
{
	for (int i = 0; i < REQUEST_MAX_PARAMS; i++) {
		if (req->params[i] == NULL) {
			req->params[i] = param;
			return 0;
		}
	}

	return -1;
}

int request_param_get(struct request *req, int index)
{

	return 0;
}

int request_cleanup(struct request *req)
{

	return 0;
}

