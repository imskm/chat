#include <unistd.h>

#include "chat.h"

#include "rescodes.h" /* Response codes in response struct */

static const char *variables[] = {
	"<channel>",
	"<command>",
	"<nick>",
	"<server>",
	"<user>",
	NULL,
};

static void prepare_error_message(struct request *req,
		const struct response *res, struct collection *col);


int response_send(int sockfd, const char *buf, size_t size)
{
	if (write(sockfd, buf, size) == -1)
		return -1;
	
	return 0;
}

int response_send_err(struct request *req, struct collection *col)
{
	const struct response *res;
	res = &responses[chat_calc_reply_index(req->status)];

	prepare_error_message(req, res, col);

	return response_send(req->src->fd, req->body, strlen(req->body));
}

void prepare_error_message(struct request *req, const struct response *res,
		struct collection *col)
{
	const char *p = NULL;

	p = res->desc;
	if (strstr(p, "<nick>") != NULL) {
		req->body = str_replace(p, "<nick>", req->dest);
		p = req->body;
	}

	if (strstr(p, "<command>") != NULL) {
		req->body = str_replace(p, "<command>", req->irc_cmd);

		/* If p is not the same as desc then p must be pointing at mallocated
		 * memory (returned by str_replace) and need to free it */
		if (p != res->desc) free((void *)p); 
		p = req->body;

	}

	if (strstr(p, "<channel>") != NULL) {
		req->body = str_replace(p, "<channel>", req->src->nick);

		/* If p is not the same as desc then p must be pointing at mallocated
		 * memory (returned by str_replace) and need to free it */
		if (p != res->desc) free((void *)p); 
		p = req->body;
	}

	/* TODO test for other variables and replace accordingly */
}

int response_send_msg(struct request *req, struct collection *col)
{

	return 0;
}

int response_send_rpl_join(struct request *req, struct collection *col)
{
	int i;
	unsigned char buf[BUFFSIZE];

	/* TODO This is non-standard, for now I am joining users and not channel
	 * 1. Delete all this code and implement the IRC standard */
	/* If target nick does not exist then response error and return */
	if ((i = chat_find_nick(col->clients, req->dest)) == -1) {
		req->status = ERR_NOSUCHNICK;
		return response_send_err(req, col);
	}
	sprintf(buf, "%d :You have joined successfully!", RPL_NONE);

	return response_send(req->src->fd, buf, strlen(buf));
}

int response_send_rpl_names(struct request *req, struct collection *col)
{
}

int response_send_rpl_nick(struct request *req, struct collection *col)
{
}

int response_send_rpl_quit(struct request *req, struct collection *col)
{
}


