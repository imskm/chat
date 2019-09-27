#include <libsocket.h>
#include <errno.h>
#include <regex.h>

#include "chat.h"
#include "str.h"
#include "easyio.h"


int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_in serveraddr;
	String username;
	unsigned char buf[BUFFSIZE] = {0};
	int username_len, ret;
	struct client client = {0};
	char errors[BUFFSIZE] = {0};

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
	client.username[0] 			= 0;
	client.is_username_set 		= false;
	client.is_assoc 			= false;

	/* Prompt for user name */
	for (; ;) {
		printf("Username(6 char max): ");
		GetString(&username, stdin);

		if (client_validate_username(username, errors) == false) {
			puts(errors);
			continue;
		}
		username_len = strlen(username);
		puts(username);


		puts("Checking username...");
		if ((ret = client_username_check(sockfd, username)) == 1) {
			fprintf(stderr, "Success: Username <%s> accepted\n", username);
			break;
		} else if (ret == -1) {
			fprintf(stderr, "[*] Username <%s> already exist, try another username\n", username);
		} else {
			fprintf(stderr, "Unable to check username <%s>\n", username);
			exit(1);
		}
	}
	strcpy(client.username, username);

	puts("[*] Fetching list of available users...");
	/* Show available online users */
	if (client_get_online_users(&client, buf, sizeof(buf)) == -1) {
		fprintf(stderr, "Failed to get available users, aborting..\n");
		goto out;
	}

	fprintf(stderr, "[*] Available online users ->\n");
	puts(buf);
	fprintf(stderr, "\n\n");

	/* Use I/O Multiplexing*/
	FD_ZERO(&allset);
	FD_SET(fileno(stdin), &allset);
	FD_SET(client.fd, &allset);
	maxfd = max(client.fd, fileno(stdin)) + 1;
	for (; ;) {
		fprintf(stderr, "[%s] ", client.username);
		rset = allset;
		nready = select(maxfd, &rset, NULL, NULL, NULL);

		/* If socket is ready for read then read */
		if (FD_ISSET(client.fd, &rset)) {
			if (client_handle_response(&client) == PEER_TERMINATED) {
				fprintf(stderr, "[!] server terminated prematurely\n");
				goto out;
			}
			if (--nready == 0)
				continue;
		}

		/* If stdin is ready then process it */
		if (FD_ISSET(fileno(stdin), &rset)) {
			client_handle_command(&client);
		}
	}

out:
	puts("[*] Closing connection..");
	close(sockfd);

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

int client_handle_command(struct client *client)
{
	int command_type;
	String command;
	char errors[BUFFSIZE];

	/* If input is empty then return (it's ok to give empty line) */
	if (GetString(&command, stdin) == 0) {
		return 0;
	}

	/* Trim leading and trailing white space */
	str_trim(command);

	command_type = client_get_command_type(command);
	switch(command_type) {

	case CLIENT_CMD_CODE_CHAT :
	{
		char username[CLIENT_USERNAME_MAX_LEN];
		char *cmd_usernameptr;

		/* +2 is for space and leading '/'  */
		cmd_usernameptr = command + CLIENT_CMD_TEXT_LEN + 2;
		
		if (!client_validate_username(cmd_usernameptr, errors)) {
			puts(errors);
			return -1;
		}
		if (client_request_assoc(client, errors, cmd_usernameptr) == -1) {
			if (errors[0] != 0)
				puts(errors);
			return -1;
		}
	}
	break;

	case CLIENT_CMD_CODE_MESG :
	{
		if (client_send_message(client, command) == -1)
			return -1;
	}
	break;

	case CLIENT_CMD_CODE_DISC :
	{
		puts("detected client_cmd_code_disc");
		//
	}
	break;

	case CLIENT_CMD_CODE_QUIT :
	{
		puts("detected client_cmd_code_quit");
		//
	}
	break;

	case CLIENT_CMD_CODE_UKNW :
	{
		puts("detected client_cmd_code_uknw");
		//
	}
	break;
		
	default :
		//
		break;
	}

	return 0;
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
