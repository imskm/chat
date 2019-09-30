#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <libsocket.h>

#include "str.h"
#include "chat.h"
#include "request.h"
#include "command.h"

const struct command commands[] = {
	{"join",    "JOIN",     command_handle_join},
	{"msg",     "PRIVMSG",  command_handle_msg},
	{"names",   "NAMES",    command_handle_names},
	{"nick",    "NICK",     command_handle_nick},
	{"quit",    "QUIT",     command_handle_quit},
	{NULL,      NULL,       NULL},
};

static int find_message_cmd_index();
static int extract_fill_params(char **cmd_cpp,
		char **params_line, int limit);

int command_handle(const char **cmd_buf)
{
	const char *start, *end;
	int len;
	
	start = *cmd_buf;

	/* Skip any leading white space in command
	 * it's nice to handle this type of command `/   command  param`
	 * in this type of command my command detection will also work */
	while (*start++)
		if (!isspace(*start))
			break;

	/* If ' ' (space) delimeter is not found then either command does not
	 * have argument or is wrong command */
	if ((end = strchr(start, ' ')) == NULL) {
		end = start + strlen(start);
	}

	/* Unknown command */
	if (end <= start)
		return -1;


	for (int i = 0; commands[i].cmd != NULL; i++) {
		len = max(end - start, strlen(commands[i].cmd));

		if (strncmp(start, commands[i].cmd, len) == 0) {
			*cmd_buf = end + 1; /* updated to point to start of next param */
			return i;
		}
	}

	return -1; /* Command unknown */
}

int	command_handle_join(struct request *req, const char *cmd_buf)
{
	int argc, ret;
	char *cmd_cp, *cmd_cpp;

	ret = 0;

	if ((cmd_cp = strdup(cmd_buf)) == NULL) {
		fprintf(stderr, "[!] command error\n");
		return -1;
	}

	str_ltrim(cmd_cp);
	cmd_cpp = cmd_cp; /* need to free it after sending request */

	/* 2= 0 param and 1 NULL */
	if ((argc = extract_fill_params(&cmd_cpp, req->params, 2)) != 1) {
		fprintf(stderr,
				"[!] number of argument missmatch, required %d given %d\n",
				1, argc);
		ret = -1;
	}
	free(cmd_cp);

	return ret;
}

int	command_handle_msg(struct request *req, const char *cmd_buf)
{
	int argc;
	char *cmd_cp, *cmd_cpp;
	char *parts[2];

	if ((cmd_cp = strdup(cmd_buf)) == NULL) {
		fprintf(stderr, "[!] command error\n");
		return -1;
	}

	str_ltrim(cmd_cp);
	cmd_cpp = cmd_cp; 

	/* Store receiver in params */
	if ((argc = extract_fill_params(&cmd_cpp, parts, 2)) != 1) {
		fprintf(stderr,
				"[!] invalid %s command, /%s <receiver> <message>\n",
				req->cmd, req->cmd);
		free(parts[0]);
		free(cmd_cp);
		return -1;
	}

	req->dest = parts[0]; /* Set the receiver */

	/* If cmd_cpp is NULL then no message was given, return error */
	if (cmd_cpp == NULL) {
		fprintf(stderr,
				"[!] No message given, /%s <receiver> <message>\n",
				req->cmd);
		free(cmd_cp);
		free(parts[0]);
		return -1;
	}

	req->body = strdup(cmd_cpp); /* Setting message as body in req */

	free(cmd_cp);

	return 0;
}

int	command_handle_names(struct request *req, const char *cmd_buf)
{
	int argc;
	char *cmd_cp, *cmd_cpp, *parts[3], buf[BUFFSIZE]; 
	
	if ((cmd_cp = strdup(cmd_buf)) == NULL) {
		fprintf(stderr, "[!] command error\n");
		return -1;
	}

	str_ltrim(cmd_cp);
	cmd_cpp = cmd_cp; /* need to free it after sending request */
	buf[0] = 0;

	/* /names command's number of arg can be 0 or at max 2 */
	argc = extract_fill_params(&cmd_cpp, parts, 3);
	for (int i = 0; i < argc; i++) {
		if (i == 0)
			sprintf(buf, "%s", parts[i]);
		else
			sprintf(buf, "%s,%s", buf, parts[i]);

		free(parts[i]);
	}

	/* TODO If cmd_cpp points to some char other that NUL terminator in other
	 * words if more than max number of params is given then print notice to
	 * user that max params for names can be given */

	req->params[0] = strdup(buf);
	req->params[1] = NULL;
	free(cmd_cp);

	return 0;
}

int	command_handle_nick(struct request *req, const char *cmd_buf)
{
	int argc;
	char *cmd_cp, *cmd_cpp, *parts[2];
	
	if ((cmd_cp = strdup(cmd_buf)) == NULL) {
		fprintf(stderr, "[!] command error\n");
		return -1;
	}

	str_ltrim(cmd_cp);
	cmd_cpp = cmd_cp; /* need to free it after sending request */

	/* /nick command's number of arg is 1 */
	if ((argc = extract_fill_params(&cmd_cpp, parts, 2)) != 1) {
		fprintf(stderr, "[!] Invalid # of args, /%s <nick>\n", req->cmd);
		free(cmd_cp);
		return -1;
	}

	req->params[0] = strdup(parts[0]);
	req->params[1] = NULL;
	free(cmd_cp);

	return 0;
}

int	command_handle_quit(struct request *req, const char *cmd_buf)
{
	puts("command_quit");

	return 0;
}

static int find_message_cmd_index()
{
	for (int i = 0; commands[i].cmd != NULL; i++) 
		if (strcmp(commands[i].cmd, "msg") == 0)
			return i;

	return -1;
}

/**
 * Finds params delimited by ' ' (space) and fills the pointer in params_line
 *  params_line need to be freed by caller
 *
 * @return int  non-negative number. Number of params successfully extracted
 */
static int extract_fill_params(char **cmd, char **params_line, int limit)
{
	int nparams 	= 0;
	char *cmdp 		= *cmd, *p;

	if (*cmdp == '\0') /* If no params is given */
		return nparams;

	while (--limit) { /* pre-decrement because limit is always +1 for NULL */
		/* If ' ' (space) not found in cmd then it must be last param or
		 * the only param therefore store this param and break out of 
		 * loop */
		if ((p = strchr(cmdp, ' ')) == NULL) {
			params_line[nparams++] = strdup(cmdp);
			cmdp = p;
			break;
		}

		*p = 0; /* NUL terminating ith param */
		params_line[nparams++] = strdup(cmdp); /* so that strdup works */
		*p = ' '; /* then restore ' ' (space) back */
		cmdp = p + 1; /* and then cmdp points to next param */

		/* Left trim any space in next param */
		str_ltrim(cmdp);
	}

	params_line[nparams] = NULL;
	*cmd = cmdp;

	return nparams;
}

int command_message_get_index()
{
	static int msg_index = -1;

	/* First char of start is not '/' (forward slash) then command is msg */
	if (msg_index == -1) {
		msg_index = find_message_cmd_index();
		return msg_index;
	}

	return -1;
}
