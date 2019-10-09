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
	unsigned char buf[BUFFSIZE];
	const struct response *res;
	int nbytes;

	res = &responses[chat_calc_reply_index(req->status)];

	prepare_error_message(req, res, col);

	nbytes = sprintf(buf, "%d %s", req->status, req->body);

	return response_send(req->src->fd, buf, nbytes);
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

int response_send_rpl_none(struct request *req, struct collection *col)
{
	unsigned char buf[BUFFSIZE];
	int nbytes;

	if (!req->body)
		req->body = strdup("Action performed");

	nbytes = sprintf(buf, "%d :%s", req->status, req->body);

	return response_send(req->src->fd, buf, nbytes);
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
	} else if (i == col->index) {
		sprintf(buf, "%d :You can't associate your self", RPL_NONE);
		return response_send(req->src->fd, buf, strlen(buf));
	}

	col->clients->clients[col->index]->partner = col->clients->clients[i];
	col->clients->clients[i]->partner = col->clients->clients[col->index];

	/* Notify the connected client */
	sprintf(buf, "%d :<%s> has requested association", RPL_NONE,
			req->src->nick);
	response_send(col->clients->clients[col->index]->partner->fd, buf,
			strlen(buf));

	sprintf(buf, "%d :You have joined successfully!", RPL_NONE);

	return response_send(req->src->fd, buf, strlen(buf));
}

int response_send_rpl_names(struct request *req, struct collection *col)
{
	char buf[BUFFSIZE];

	sprintf(buf, "%d ", RPL_NAMREPLY);
	chat_serialize_nick(col->clients, buf + 4, sizeof(buf) - 4);

	/* TODO Handle the case when buf is full but more nick is remaining */

	response_send(req->src->fd, buf, strlen(buf));
	sprintf(buf, "%d", RPL_ENDOFNAMES);

	return response_send(req->src->fd, buf, strlen(buf));
}

int response_send_rpl_nick(struct request *req, struct collection *col)
{
}

int response_send_rpl_quit(struct request *req, struct collection *col)
{
}


