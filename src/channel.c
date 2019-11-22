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

