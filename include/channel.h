#ifndef __CHANNEL_H
#define __CHANNEL_H 

#include <stdbool.h>

#define CHANNEL_NAME_MAX_LEN   9
#define CHANNEL_TOPIC_MAX_LEN  20
#define CHANNEL_KEY_MAX_LEN    10
#define CHANNEL_USERS_MAX_LEN  10
#define CHANNEL_MAX_LEN  10

struct channel {
	char           channel_name[CHANNEL_NAME_MAX_LEN + 1];
	char           topic[CHANNEL_TOPIC_MAX_LEN];
	char           key[CHANNEL_KEY_MAX_LEN];
	struct client  *connected_users[CHANNEL_USERS_MAX_LEN];
	int            total_connected_users;
	unsigned int   attr;

};

struct channels {
	struct channel *channels[CHANNEL_MAX_LEN];
	int            nchannels;
}

int channel_create(struct channels *channels);
int channel_delete(struct channels *channels, int index);

int channel_attr_set(struct *channel, unsigned int attr);
int channel_attr_unset(struct *channel, unsigned int attr);
bool channel_attr_isset(struct *channel, unsigned int attr);

void channel_topic_set(struct *channel, const char *topic);
bool channel_topic_isset(struct *channel);
void channel_topic_unset(struct *channel);

int channel_key_set(struct *channel, const char *key);







#endif