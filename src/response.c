#include <unistd.h>

#include "chat.h"

#include "rescodes.h" /* Response codes in response struct */

static void prepare_error_message(struct request *req,
		const struct response *res, struct collection *col);

static int response_send_msg_channel(struct request *req, struct collection *col);
static int response_send_msg_client(struct request *req, struct collection *col);


int response_send(int sockfd, const char *buf, size_t size)
{
	char msg_buf[BUFFSIZE];
	int nbytes;

	if (buf[0] != ':') {
		nbytes = sprintf(msg_buf, "%s%s", ":server ", buf);
	} else {
		if (write(sockfd, buf, size) == -1)
			return -1;
		return 0;
	}
	
	if (write(sockfd, msg_buf, nbytes) == -1)
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

	nbytes = sprintf(buf, ":server %d %s %s", req->status, req->src->nick,
			req->body);

	return response_send(req->src->fd, buf, strlen(buf));
}

void prepare_error_message(struct request *req, const struct response *res,
		struct collection *col)
{
	const char *p = NULL;

	p = res->desc;
	if (strstr(p, "<nick>") != NULL && req->params[0] != NULL) {
		req->body = str_replace(p, "<nick>", req->params[0]);
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

int response_send_msg_channel(struct request *req, struct collection *col)
{
	int index, buf_size;
	struct channel *channel;
	unsigned char buf[BUFFSIZE];

	index = chat_find_channelname(col->channels, req->params[0]);
	channel = col->channels->channels[index];

	/* Send the message to recipients */
	sprintf(buf, ":%s %s %s :%s", req->src->nick, req->irc_cmd, req->params[0],
				req->body);

	buf_size = strlen(buf);
	for (int i = 0; i < channel->total_connected_users; ++i)
	{
		if (channel->connected_users[i] == NULL)
			continue;

		if (channel->connected_users[i] == req->src)
			continue;
		
		// Todo : log error msg if response_send() returns -1
		response_send(channel->connected_users[i]->fd, buf, buf_size);
	}

	return 0;
}

int response_send_msg_client(struct request *req, struct collection *col)
{
	int i;
	struct client *recipient;
	unsigned char buf[BUFFSIZE];

	/* Find recipient */
	i = chat_find_nick(col->clients, req->params[0]);
	recipient = col->clients->clients[i];

	/* Send the message to recipient */
	sprintf(buf, ":%s %s %s :%s", req->src->nick, req->irc_cmd, req->params[0],
			req->body);
	return response_send(recipient->fd, buf, strlen(buf));
}

int response_send_msg(struct request *req, struct collection *col)
{
	int i;
	unsigned char buf[BUFFSIZE];

	if (req->params[0][0] == '#')
		response_send_msg_channel(req, col);

	else 
		response_send_msg_client(req, col);

	/* Send the same message to the sender */
	sprintf(buf, ":%s %s %s :%s", req->src->nick, req->irc_cmd, req->src->nick,
			req->body);

	return response_send(req->src->fd, buf, strlen(buf)) && i;
}

int response_send_rpl_welcome(struct request *req, struct collection *col)
{
	char buf[BUFFSIZE];

	/* 5. Send the same message to the sender */
	sprintf(buf, "%03d %s :%s", RPL_WELCOME, req->src->nick, req->body);

	return response_send(req->src->fd, buf, strlen(buf));
}

int response_send_rpl_none(struct request *req, struct collection *col)
{
	unsigned char buf[BUFFSIZE];
	int nbytes;

	if (!req->body)
		req->body = strdup("Action performed");

	nbytes = sprintf(buf, "%d %s :%s", req->status, req->src->nick, req->body);

	return response_send(req->src->fd, buf, strlen(buf));
}

int response_send_rpl_join(struct request *req, struct collection *col)
{
	/* @TODO Previous implementation was non-standard.
	 * Implement channel joining here */

	unsigned char buf[BUFFSIZE];
	char *msg;
	int index;
	request_dump(req);
	printf("Status: %d\n", req->status);

	if (req->status == RPL_NOTOPIC) {
		msg = responses[chat_calc_reply_index(req->status)].desc;
	} else if (req->status == RPL_TOPIC) {
		index = chat_find_channelname(col->channels, req->params[0]);
		msg = col->channels->channels[index]->topic;
	}
	sprintf(buf, "%d %s :%s", req->status, req->src->nick, msg);
	response_send(req->src->fd, buf, strlen(buf));

	return response_send_rpl_names(req, col);
}

int response_send_rpl_names(struct request *req, struct collection *col)
{
	int index;
	char buf[BUFFSIZE];

	// TODO: found a bug when names command is given without any arguments(channelname)
	// it genenrates sigsegv 
	index = chat_find_channelname(col->channels, req->params[0]);
	if (index == -1) {
		req->status = ERR_NOSUCHCHANNEL;
		return -1;
	}

	struct client **clients = col->channels->channels[index]->connected_users;

	for (int i = 0; i < col->channels->channels[index]->total_connected_users; i++) {
		if (clients[i] == NULL)
			continue;
		fprintf(stderr, "----> %s\n", clients[i]->nick);
	}

	sprintf(buf, "%d %s :", RPL_NAMREPLY, req->src->nick);
	chat_serialize_nick(
		col->channels->channels[index]->connected_users,
		col->channels->channels[index]->total_connected_users,
		buf + strlen(buf),
		sizeof(buf) - strlen(buf)
	);




	/* TODO Handle the case when buf is full but more nick is remaining */

	response_send(req->src->fd, buf, strlen(buf));
	sprintf(buf, "%d %s", RPL_ENDOFNAMES, req->src->nick);

	return response_send(req->src->fd, buf, strlen(buf));
}

int response_send_rpl_nick(struct request *req, struct collection *col)
{
}

int response_send_rpl_quit(struct request *req, struct collection *col)
{
	//char buf[BUFFSIZE];

	// SEGVINT If src is not associated with partner then server will crash
	//sprintf(buf, ":%s QUIT %s", req->src->nick, req->src->partner->nick);

	//return response_send(req->src->partner->fd, buf, strlen(buf));
	return 0;
}

int response_send_rpl_part(struct request *req, struct collection *col)
{
	// Todo: broadcast the leave message to the channel
	
	char msg[BUFFSIZE];
	sprintf(msg, "User %s has left the channel", req->src->nick);
	req->body = strdup(msg);

	return response_send_msg_channel(req, col);
}

