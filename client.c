#include <errno.h>
#include <regex.h>
#include <stdbool.h>
#include <termios.h>
#include <time.h>

#include <libsocket.h>
#include <cursor.h>

#include "chat.h"
#include "command.h"
#include "str.h"

struct text_view {
	size_t index;
	unsigned char buf[BUFFSIZE];
};

int cui_textview(struct text_view *textv);
void client_render_cmdline(char *status_line, char *prompt, char *typed_cmd);
char *client_construct_info_line(const char *info, char *out_line);

bool is_quit = false;


int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_in serveraddr;
	unsigned char buf[BUFFSIZE] = {0};
	int username_len, ret;
	struct client client = {0};
	char errors[BUFFSIZE] = {0};
	struct termios term_orig, term;
	struct text_view text_view = {0};

	char status_line[256], prompt[256]; 

	int maxfd, nready;
	fd_set rset, allset;


	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVER_PORT);
	Inet_pton(AF_INET, SERVER_IP, &serveraddr.sin_addr);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	Connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(struct sockaddr));

	/* Setup client */
	client.fd 					= sockfd;
	client.pair_fd 				= -1;
	client.is_username_set 		= false;
	client.is_assoc 			= false;
	client.pair[0]				= 0;
	strcpy(client.nick, "?????????");

	/* Setup terminal ios */
	if (tcgetattr(STDIN_FILENO, &term_orig) == -1) {
		perror("tcgetattr error");
		exit(1);
	}
	term = term_orig;
	term.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &term);
	cur_tobottom();
	cur_toleft();

	/* Use I/O Multiplexing*/
	FD_ZERO(&allset);
	FD_SET(fileno(stdin), &allset);
	FD_SET(client.fd, &allset);
	maxfd = max(client.fd, fileno(stdin)) + 1;
	for (; ;) {
		sprintf(prompt, "[%s]", client.nick);
		client_render_cmdline(status_line, prompt, text_view.buf);
		rset = allset;
		nready = select(maxfd, &rset, NULL, NULL, NULL);

		/* If socket is ready for read then read */
		if (FD_ISSET(client.fd, &rset)) {
			if (chat_response_handle(&client) == PEER_TERMINATED) {
				client_info_printline("Server terminated prematurely");
				goto out;
			}
			if (--nready == 0)
				continue;
		}

		/* If stdin is ready then process it (user command) */
		if (FD_ISSET(fileno(stdin), &rset)) {
			if (cui_textview(&text_view) == '\n' && text_view.index != 0) {
				chat_command_handle(&client, text_view.buf);
				text_view.index = 0;
				text_view.buf[0] = 0;
			}
		}
	}

out:
	client_info_printline("Closing connection...");
	close(sockfd);
	tcsetattr(STDIN_FILENO, TCSANOW, &term_orig);

	return 0;
}

int client_username_check(int sockfd, const char *username)
{
	int msg_len, nbytes;
	unsigned char req_buf[BUFFSIZE], res_buf[BUFFSIZE];

	msg_len = sprintf(req_buf, REQUEST_USERN_TEXT "%s", username);
	req_buf[msg_len] = 0;
	fprintf(stderr, "%s\n", req_buf);

	for (; ;) {
		if ((nbytes = write(sockfd, req_buf, msg_len)) < 0) {
			perror("write error");
			return 0;
		}

		fprintf(stderr, "Waiting for username check response...\n");
		if ((nbytes = read(sockfd, res_buf, sizeof(res_buf))) < 0) {
			perror("read error");
			return 0;
		}

		fprintf(stderr, "Username check response came\n");
		res_buf[nbytes] = 0;
		if (strcmp(res_buf, RESPONSE_USERN_OK) == 0)
			break;
		else if (strcmp(res_buf, RESPONSE_USERN_ERR) == 0)
			return -1;
	}

	return 1;
}

int	client_get_online_users(struct client *client, unsigned char *buf,
		size_t size)
{
	size_t nbytes;

	if (write(client->fd,
				REQUEST_AVAIL_TEXT, REQUEST_TEXT_LEN) < 0) {
		perror("client_get_online_users: write error");
		return -1;
	}

	if ((nbytes = read(client->fd, buf, size - 1)) == -1) {
		perror("client_get_online_users: read error");
		return -1;
	} else if (nbytes == 0) {
		fprintf(stderr, "[*] Server terminated prematurely\n");
		return -1;
	}

	buf[nbytes] = 0;

	return 0;
}

bool client_validate_username(const char *username, char *errors)
{
	char regexstr[32];
	int len;
	regex_t regex;
	bool ret = false;

	/* Username validation */
	if ((len = strlen(username)) > CLIENT_USERNAME_MAX_LEN) {
		sprintf(errors, "[*] Invalid username: max %d characters allowed",
				CLIENT_USERNAME_MAX_LEN);
		return false;
	}
	sprintf(regexstr, "^[a-zA-Z0-9]{1,%d}$", CLIENT_USERNAME_MAX_LEN);

	if (regcomp(&regex, regexstr, REG_EXTENDED) != 0) {
		fprintf(stderr, "[*] client_validate_username: regcomp error\n");
		goto out;
	}

	if (regexec(&regex, username, 0, NULL, 0) == REG_NOMATCH) {
		strcpy(errors, "[*] Invalid username: Onlye a-Z, A-Z and 0-9 "
				"characters are llowed");
		goto out;
	}
	ret = true;

out:
	regfree(&regex);

	return ret;
}

int	client_request_assoc(struct client *client, char *errors,
		const char *username) {
	char buf[BUFFSIZE] = {0};
	char res_buf[BUFFSIZE] = {0};
	int nbytes;

	errors[0] = 0;
	nbytes = sprintf(buf, "%s%s", REQUEST_ASSOC_TEXT, username);

	/* Request for assocciation */
	if ((nbytes = write(client->fd, buf, nbytes)) == -1) {
		perror("[!] client_request_assoc: write error");
		return -1;
	} else if (nbytes == 0) {
		fprintf(stderr, "[!] client_request_assoc: no data sent\n");
		return -1;
	}

	/* Handle the assocciation response */
	/*
	if ((nbytes = read(client->fd, buf, sizeof(buf) - 1)) == -1) {
		perror("[!] client_request_assoc: read error");
		return -1;
	} else if (nbytes == 0) {
		fprintf(stderr,
				"[!] client_request_assoc: server terminated prematurely\n");
		return -2;
	}
	*/

	/* Check for error */
	buf[nbytes] = 0;
	if (strncmp(buf, RESPONSE_RESER, sizeof(RESPONSE_RESER) - 1) == 0) {
		strncpy(errors, buf + sizeof(RESPONSE_RESER) - 1, BUFFSIZE - 1);
		errors[BUFFSIZE - 1] = 0;
		return -1;
	}

	return 0;
}

int	client_handle_response(struct client *client)
{
	char buf[BUFFSIZE] = {0};
	int nbytes;

	/* If response has error then return */
	if ((nbytes = read(client->fd, buf, sizeof(buf) - 1)) == -1) {
		perror("[!] client_handle_response: read error");
		return -1;
	} else if (nbytes == 0) {
		fprintf(stderr, "[!] server terminated prematurely\n");
		return PEER_TERMINATED;
	}
	buf[nbytes] = 0;
	if (strncmp(buf, RESPONSE_RESER, sizeof(RESPONSE_RESER) - 1) == 0) {
		fprintf(stderr, "[!] %s\n", buf + sizeof(RESPONSE_RESER) - 1);
		return -1;
	}

	/* If response is not OK then unkown error occured, return */
	if (strncmp(buf, RESPONSE_RESOK, sizeof(RESPONSE_RESOK) - 1)) {
		fprintf(stderr, "[!] unkown response came from server\n");
		return -1;
	}

	/* Response OK, print the message sent by server */
	puts(buf + sizeof(RESPONSE_RESOK) - 1);

	return 1;
}

int	client_get_command_type(const char *cmd)
{
	int len, cmd_code;
	char cmd_part[CLIENT_CMD_TEXT_LEN + 1];
	char cmd_matched[CLIENT_CMD_TEXT_LEN + 1] = {0};

	len = strlen(cmd);
	cmd_code = CLIENT_CMD_CODE_UKNW;

	/* If '/' is not first char then it is a message */
	if (cmd[0] != '/')
		return CLIENT_CMD_CODE_MESG;

	/* If cmd is unknown then return command unknown code
	 *  -1 for leading '/'
	 */
	if (len - 1 < CLIENT_CMD_TEXT_LEN)
		return CLIENT_CMD_CODE_UKNW;

	/* +1 for excluding leading '/' */
	strncpy(cmd_part, cmd + 1, CLIENT_CMD_TEXT_LEN);
	cmd_part[CLIENT_CMD_TEXT_LEN] = 0;
	str_toupper(cmd_part);

	if (strncmp(CLIENT_CMD_TEXT_CHAT, cmd_part, CLIENT_CMD_TEXT_LEN) == 0) {
		strncpy(cmd_matched, cmd_part, CLIENT_CMD_TEXT_LEN);
		cmd_code = CLIENT_CMD_CODE_CHAT;
	}
	else if (strncmp(CLIENT_CMD_TEXT_DISC, cmd_part, CLIENT_CMD_TEXT_LEN) == 0) {
		strncpy(cmd_matched, cmd_part, CLIENT_CMD_TEXT_LEN);
		cmd_code = CLIENT_CMD_CODE_DISC;
	}
	else if (strncmp(CLIENT_CMD_TEXT_QUIT, cmd_part, CLIENT_CMD_TEXT_LEN) == 0) {
		strncpy(cmd_matched, cmd_part, CLIENT_CMD_TEXT_LEN);
		cmd_code = CLIENT_CMD_CODE_QUIT;
	}

	/* +1 for cosidering leading '/' in cmd */
	if (cmd[CLIENT_CMD_TEXT_LEN + 1] != ' ') {
		cmd_matched[CLIENT_CMD_TEXT_LEN] = 0;
		fprintf(stderr, "[!] command unknown, did you mean %s\n", cmd_matched);
		return CLIENT_CMD_CODE_UKNW;
	}

	return cmd_code;
}

int	client_send_message(struct client *client, const char *msg)
{
	char res_buf[BUFFSIZE];
	int nbytes;

	nbytes = sprintf(res_buf, "%s%s", REQUEST_MESSG_TEXT, msg);

	if (write(client->fd, res_buf, nbytes) == -1) {
		perror("client_send_message: write error");
		return -1;
	}


	return 0;
}

int cui_textview(struct text_view *textv)
{
	int ch;

	/* TODO Handle Escape control characters */
	if ((ch = getchar()) == '\n' && textv->index == 0)
		return ch;
	else if (ch == 127 && textv->index == 0)
		return ch;

	switch (ch) {
		case 127: textv->index--; break;
		case '\n': break;
		default  : textv->buf[textv->index++] = ch; break;
	}

	textv->buf[textv->index] = 0;

	return ch;
}

void client_render_cmdline(char *status_line, char *prompt, char *typed_cmd)
{
	const int width = 80;

	cur_up(1);
	cur_toleft();
	fprintf(stdout, "\033[37;104m%-*s\033[0m\n", width, "[STATUS LINE]");
	fprintf(stdout, "\033[0K%s %s", prompt, typed_cmd);
	fflush(stdout);
}

void client_print_line(const char *line)
{
	cur_up(1);
	cur_toleft();
	fprintf(stdout, "\033[0K"); /* Clear line */
	cur_toleft();
	fprintf(stdout, "%s\n\n", line);
}

char *client_construct_info_line(const char *info, char *out_line)
{
	struct tm *tm;
	time_t t;

	if ((t = time(NULL)) == ((time_t) -1))
		return NULL;
	tm = localtime(&t);
	sprintf(out_line,
			"%02d:%02d:%02d\033[1m\033[31m%13s\033[0m \033[32m| "
			"\033[31m%s\033[0m", tm->tm_hour, tm->tm_min, tm->tm_sec,
			"[*]", info);

	return out_line;
}

void client_info_printline(const char *line)
{
	char log_msgline[256];

	client_construct_info_line(line, log_msgline);
	client_print_line(log_msgline);
}
