#ifndef __REQUEST_H
#define __REQUEST_H

#define REQUEST_MAX_PARAMS 9 /* 8 +1 is for NULL terminated */

struct request {
	int   type;
	const char *src;   	/* nick of source  */
	const char *dest;  	/* nick of destination  */
	const char *cmd;   	/* name of command (client side command) */
	const char *irc_cmd;/* name of command (IRC protocol command) */
	/* parameters of command message (NULL terminated) */
	char *params[REQUEST_MAX_PARAMS];
	char *body;	  		/* body of the request (message) */
	int   status;		/* will be used for response */
};

int request_param_set(struct request *req, char *param);
int request_param_get(struct request *req, int index);
int request_cleanup(struct request *req);

#endif
