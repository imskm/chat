#ifndef __CHANNEL_H
#define __CHANNEL_H 

#include <stdbool.h>

#include "chat.h"

#define CHANNEL_NAME_MAX_LEN   9
#define CHANNEL_TOPIC_MAX_LEN  20
#define CHANNEL_KEY_MAX_LEN    10
#define CHANNEL_USERS_MAX_LEN  10
#define CHANNEL_MAX_LEN  10

struct channel {
	char           channelname[CHANNEL_NAME_MAX_LEN + 1];
	char           topic[CHANNEL_TOPIC_MAX_LEN];
	char           key[CHANNEL_KEY_MAX_LEN];
	struct client  *connected_users[CHANNEL_USERS_MAX_LEN];
	int            operator_index;
	int            total_connected_users;
	unsigned int   attr;

};

struct channels {
	struct channel *channels[CHANNEL_MAX_LEN];
	int            nchannels;
};

int  channel_create(struct request *req, struct collection *collection);
int  channel_delete(struct channels *channels, int index);

int  channel_attr_set(struct channel *channel, unsigned int attr);
int  channel_attr_unset(struct channel *channel, unsigned int attr);
bool channel_attr_isset(struct channel *channel, unsigned int attr);

void channel_topic_set(struct channel *channel, const char *topic);
bool channel_topic_isset(struct channel *channel);
void channel_topic_unset(struct channel *channel);

int  channel_key_set(struct channel *channel, const char *key);
void channel_dump(struct channel *channel);
int  channel_is_user_connected(struct channel *channel, struct client *client);

int  channel_user_remove(struct request *req, struct collection *collection);






#endif