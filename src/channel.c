#include <stdio.h>

#include "channel.h"

int channel_create(struct request *req, struct collection *collection)
{
	struct channel *channel;	
	if ((channel = malloc(sizeof(struct channel))) == NULL) {
		perror("channel_create : malloc error");
		return -1;
	}
	
	/* Set default values */
	strcpy(channel->channelname, req->params[0]);
	channel->topic[0] 			    = 0;
	channel->key[0] 		        = 0;
	channel->connected_users[0] 	= req->src;
	channel->total_connected_users	= 1;
	channel->operator_index         = 0;

	collection->channels->channels[collection->channels->nchannels++] = channel;

	return 0;
}

void channel_dump(struct channel *channel)
{
	fprintf(stderr, "Channel Name  : %s\n", channel->channelname);
	fprintf(stderr, "Channel Topic : %s\n", channel->topic);
	fprintf(stderr, "Users :%s\n", channel->topic);

	for (int i = 0; i < channel->total_connected_users; ++i)
	{
		if (channel->connected_users[i] == NULL)
			continue;

		fprintf(stderr, "          %s\n", channel->connected_users[i]->nick);
	}
}

int channel_is_user_connected(struct channel *channel, struct client *client)
{
	if (channel == NULL || client == NULL)
		goto out;

	for (int i = 0; i < channel->total_connected_users; ++i)
	{
		if (channel->connected_users[i] == NULL)
			continue;

		if (channel->connected_users[i] == client)
			return i;
		
	}
	
	goto out;

out :

	return -1;
}

int  channel_user_remove(struct request *req, struct collection *col)
{
	int index, i;

	index = chat_find_channelname(col->channels, req->params[0]);

	i = channel_is_user_connected(col->channels->channels[index], req->src);

	if (i == -1) 
		return -1;

	col->channels->channels[index]->connected_users[i] = NULL;

	return 0;
}
