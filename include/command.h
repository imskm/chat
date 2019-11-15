#ifndef __COMMAND_H
#define __COMMAND_H

struct command {
	const char *cmd;
	const char *irc_cmd;
	int  (*handle)(struct request *, const char *); /* last arg is buf */
	int  (*request)(struct request *, struct collection *);
};

extern const struct command commands[];

int		command_handle(const char **cmd_buf);

/* Theses function correctly populate req struct as each command is required
 * using buf. The struct req can be used for other socket communication app
 * for my next projects.
 * Here each command is responsible for parsing command using buf and
 * populating the req struct accordingly.
 * Whenever new function is added in command_handle_ then it must be added in
 * commands struct variable above */
int		command_handle_join(struct request *req, const char *cmd_buf);
int		command_handle_msg(struct request *req, const char *cmd_buf);
int		command_handle_names(struct request *req, const char *cmd_buf);
int		command_handle_nick(struct request *req, const char *cmd_buf);
int		command_handle_quit(struct request *req, const char *cmd_buf);

int 	command_message_get_index();


#endif
