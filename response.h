#ifndef __RESPONSE_H
#define __RESPONSE_H

/* This response struct is for only responsing to client's command when client
 * sends wrong IRC protocol command or server needs to send some data back to
 * client in response to  client's request (example will be NAMES command.
 * **This sturct will be used to send response to client who sends request
 *   to server */
struct response {
	int code;
	char *desc;
	int (*handle)(struct request *req, struct collection *col);
};

extern const struct response responses[];

int response_err_send(struct request *req, struct collection *col);
int response_send(int sockfd, const char *buf, size_t size);

#endif
