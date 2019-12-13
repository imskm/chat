#include <errno.h>
#include <regex.h>
#include <stdbool.h>
#include <termios.h>

#include <libsocket.h>
#include <cursor.h>

#include "chat.h"
#include "command.h"
#include "str.h"

#define MAX_CHANNEL_CONNECTION_ALLOWED 5

struct text_view {
	size_t index;
	unsigned char buf[BUFFSIZE];
};

int cui_textview(struct text_view *textv);
void client_render_cmdline(char *status_line, char *prompt, char *typed_cmd);

bool is_quit = false;

static struct client client = {0};
char *conn_channels[MAX_CHANNEL_CONNECTION_ALLOWED];

int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_in serveraddr;
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
	for(; ;) {
		sprintf(prompt, "[%s]", client.nick);
		client_render_cmdline(status_line, prompt, text_view.buf);
		rset = allset;
		nready = select(maxfd, &rset, NULL, NULL, NULL);

		/* If socket is ready for read then read */
		if (FD_ISSET(client.fd, &rset)) {
			if (chat_response_handle(&client) == PEER_TERMINATED) {
				chat_info_printline("Server terminated prematurely");
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

		if (is_quit)
			break;
	}

	chat_info_printline("Sent quit request to server");

out:
	chat_info_printline("Closing connection...");
	close(sockfd);
	tcsetattr(STDIN_FILENO, TCSANOW, &term_orig);

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

int client_nick_update(const char *nick)
{
	strncpy(client.nick, nick, CLIENT_USERNAME_MAX_LEN);
	client.nick[CLIENT_USERNAME_MAX_LEN] = 0;

	return 0;
}

void client_quit_set()
{
	is_quit = true;
}
