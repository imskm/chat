# Chat App
IRC RFC Documentation: https://tools.ietf.org/html/rfc1459
Building an IRC server/client. Current implementation of IRC server/client on web does not support one-one communication in connected and private fashion.
I am building this IRC server/client to add this feature including all the features currently supported by IRC protocol. But mine will run on single server and all client will send to and receive message from my single server.
Features that will work on my version of IRC server/client application:
* All the current implementation of IRC server/client application
* Will allow user to connect with another user to exchange message privately and they will be isolated from rest of the users. While they are exchanging messages they can receive messages from other users and also can reply to them but the user need to use `/msg` command to reply or send message to specific user.
* When a user is associated with other user using `/join <nick>` command then he can just type the message and the message will be sent to to associated user at the other end.
* User can change their visibility. This feature will allow them to set them selves invisible while chatting privately with one-one.

* Client nick name maximum lenth is 9 chars
* Channel names are string (begining with `&` and `#`) of length upto 200 chars
* To create a new channel, user just joins that channel, if chanel exist then he just joins it else new channel with the channel name is created and the user becomes channel operator.
* IRC support invitation system for joining a channel. If a channel is created with invite only mode then only ivited user can join that channel

The commands which may only be used by channel operators are:
```
KICK    - Eject a client from the channel
MODE    - Change the channel's mode
INVITE  - Invite a client to an invite-only channel (mode +i)
TOPIC   - Change the channel topic in a mode +t channel
```

* Channel operator is identified by `@` symbol next to their nick name
* IRC support
	- One-to-one
	- One-to-many
		- To-a-list
		- To-a-group
		- To-a-host/server
	- One-to-all (broadcast)
	- Client-to-client
	- Client-to-server
	- Server-to-server

## General structre of message
```
   The BNF representation for this is:


<message>  ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
<prefix>   ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
<command>  ::= <letter> { <letter> } | <number> <number> <number>
<SPACE>    ::= ' ' { ' ' }
<params>   ::= <SPACE> [ ':' <trailing> | <middle> <params> ]

<middle>   ::= <Any *non-empty* sequence of octets not including SPACE
               or NUL or CR or LF, the first of which may not be ':'>
<trailing> ::= <Any, possibly *empty*, sequence of octets not including
                 NUL or CR or LF>

<crlf>     ::= CR LF
```

## Message details
* The server is required to parse the complete message and returning any appropriate errors to connected client.
* A fatal error must be sent to client if client sends invalid command and server encounter while parsing the message
* In the examples below, some messages appear using the full format:
```
   :Name COMMAND parameter list
```
 Such examples represent a message from "Name" in transit between servers, where it is essential to include the name of the original sender of the message so remote servers may send back a reply along the correct path.

## Connection registration
1. Pass message
2. Nick message
3. User message

1. Pass message
 Command: PASS
    Parameters: <password>
```
PASS secretpasswordhere
```
Server response:
```
ERR_NEEDMOREPARAMS              ERR_ALREADYREGISTRED
```
2. Nick message
   Command: NICK
Parameters: <nickname> [ <hopcount> ]
NICK message is used to give user a nickname or change the previous one.
Numeric Replies:
```
ERR_NONICKNAMEGIVEN             ERR_ERRONEUSNICKNAME
ERR_NICKNAMEINUSE               ERR_NICKCOLLISION

Example:

NICK Wiz                        ; Introducing new nick "Wiz".

:WiZ NICK Kilroy                ; WiZ changed his nickname to Kilroy.
```
3. User message
The USER message is used at the beginning of connection to specify the username, hostname, servername and realname of the new user.
Command: USER
   Parameters: <username> <hostname> <servername> <realname>
Numeric Replies:
```
ERR_NEEDMOREPARAMS              ERR_ALREADYREGISTRED

Examples:


USER guest tolmoon tolsun :Ronnie Reagan
								 ; User registering themselves with a
								 username of "guest" and real name
								 "Ronnie Reagan".

```

## Quit
A client session is ended with a quit message.
   Command: QUIT
Parameters: [<Quit message>]
```
Numeric Replies:

None.

Examples:

QUIT :Gone to have lunch        ; Preferred message format.
```

## Server quit message
   Command: SQUIT
Parameters: <server> <comment>
The <comment> is also filled in by servers which may place an error or similar message here.
Note:
Similarly, a QUIT message must be sent to the other connected servers rest of the network on behalf of all clients behind that link.  In addition to this, all channel members of a channel which lost a member due to the split must be sent a QUIT message.

If a server connection is terminated prematurely (e.g. the server  on the  other  end  of  the  link  died),  the  server  which  detects this disconnection is required to inform the rest of  the  network that  the connection  has  closed  and  fill  in  the comment field with something appropriate.

```
Numeric replies:

        ERR_NOPRIVILEGES                ERR_NOSUCHSERVER

Example:

SQUIT tolsun.oulu.fi :Bad Link ? ; the server link tolson.oulu.fi has
                                   been terminated because of "Bad Link".

:Trillian SQUIT cm22.eng.umd.edu :Server out of control
                                    ; message from Trillian to disconnect
                                   "cm22.eng.umd.edu" from the net
                                    because "Server out of control".
```

## Join message
The JOIN command is used by client to start listening a specific channel.
The conditions which affect this are as follows:
	1. the user must be invited if the channel is invite-only;
	2.  the user's nick/username/hostname must not match any active bans;
	3.  the correct key (password) must be given if it is set.

Command: JOIN
Parameters: <channel>{,<channel>} [<key>{,<key>}]

If a JOIN is successful, the user is then sent the channel's topic (using RPL\_TOPIC) and the list of users who are on the channel (using RPL\_NAMREPLY), which must include the user joining.

Numeric Replies:
```
           ERR_NEEDMOREPARAMS              ERR_BANNEDFROMCHAN
           ERR_INVITEONLYCHAN              ERR_BADCHANNELKEY
           ERR_CHANNELISFULL               ERR_BADCHANMASK
           ERR_NOSUCHCHANNEL               ERR_TOOMANYCHANNELS
           RPL_TOPIC

   Examples:

   JOIN #foobar                    ; join channel #foobar.

   JOIN &foo fubar                 ; join channel &foo using key "fubar".

   JOIN #foo,&bar fubar            ; join channel #foo using key "fubar"
                                   and &bar using no key.

   JOIN #foo,#bar fubar,foobar     ; join channel #foo using key "fubar".
                                   and channel #bar using key "foobar".

   JOIN #foo,#bar                  ; join channels #foo and #bar.

   :WiZ JOIN #Twilight_zone        ; JOIN message from WiZ
```

## Part message
The PART message causes the client sending the message to be removed from the list of active users for all given channels listed in the parameter string.
   Command: PART
Parameters: <channel>{,<channel>}

```
Numeric Replies:

           ERR_NEEDMOREPARAMS              ERR_NOSUCHCHANNEL
           ERR_NOTONCHANNEL

   Examples:

   PART #twilight_zone             ; leave channel "#twilight_zone"

   PART #oz-ops,&group5            ; leave both channels "&group5" and
                                   "#oz-ops".
```

## Sending message
RIVMSG and NOTICE are the only messages available which actually perform delivery of a text message from one client to another.

* Private message
   Command: PRIVMSG
Parameters: <receiver>{,<receiver>} <text to be sent>

```
 Numeric Replies:

           ERR_NORECIPIENT                 ERR_NOTEXTTOSEND
           ERR_CANNOTSENDTOCHAN            ERR_NOTOPLEVEL
           ERR_WILDTOPLEVEL                ERR_TOOMANYTARGETS
           ERR_NOSUCHNICK
           RPL_AWAY

   Examples:

:Angel PRIVMSG Wiz :Hello are you receiving this message ?
                                ; Message from Angel to Wiz.

PRIVMSG Angel :yes I'm receiving it !receiving it !'u>(768u+1n) .br ;
                                Message to Angel.

PRIVMSG jto@tolsun.oulu.fi :Hello !
                                ; Message to a client on server
								tolsun.oulu.fi with username of "jto".
```

* Notice message
The NOTICE message is used similarly to PRIVMSG.  The difference between NOTICE and PRIVMSG is that automatic replies must never be sent in response to a NOTICE message.
   Command: NOTICE
Parameters: <nickname> <text>

## User based query

* The WHO message is used by a client to generate a query which returns a list of information which 'matches' the <name> parameter given by the client.  In the absence of the <name> parameter, all visible (users who aren't invisible (user mode +i) and who don't have a common channel with the requesting client) are listed.

Numeric Replies:
```
           ERR_NOSUCHSERVER
           RPL_WHOREPLY                    RPL_ENDOFWHO

   Examples:

   WHO *.fi                        ; List all users who match against
                                   "*.fi".

   WHO jto* o                      ; List all users with a match against
                                   "jto*" if they are an operator.
```

### Away message
With the AWAY message, clients can set an automatic reply string for any PRIVMSG commands directed at them (not to a channel they are on).  The automatic reply is sent by the server to client sending the PRIVMSG command.
   Command: AWAY
Parameters: [message]

Numeric Replies:
```
           RPL_UNAWAY                      RPL_NOWAWAY

   Examples:

   AWAY :Gone to lunch.  Back in 5 ; set away message to "Gone to lunch.
                                   Back in 5".

   :WiZ AWAY                       ; unmark WiZ as being away.
```

### Users message
The USERS command returns a list of users logged into the server in a similar  format  to  who(1)

Numeric Replies:
```
           ERR_NOSUCHSERVER                ERR_FILEERROR
           RPL_USERSSTART                  RPL_USERS
           RPL_NOUSERS                     RPL_ENDOFUSERS
           ERR_USERSDISABLED

   Disabled Reply:

           ERR_USERSDISABLED

   Examples:

USERS eff.org                   ; request a list of users logged in on
                                server eff.org

:John USERS tolsun.oulu.fi      ; request from John for a list of users
                                logged in on server tolsun.oulu.fi
```

## Replies
Reply is the server response. Server replies with error when an error occurs on user request. These are the following replies which are generated in response to the command given above (see https://tools.ietf.org/html/rfc1459 for entire commands that client sends to server)

### Error replies
There are many error replies therefore I can not so them all here but few are:

401     ERR_NOSUCHNICK "<nickname> :No such nick/channel"
	- Used to indicate the nickname parameter supplied to a
	command is currently unused.
402     ERR_NOSUCHSERVER "<server name> :No such server"
    - Used to indicate the server name given currently doesn't exist.

...
**see the https://tools.ietf.org/html/rfc1459 for all error replies
Note: all the error is grouped in `400 and 500` range**

### Command replies
300     RPL_NONE Dummy reply number. Not used.

302     RPL_USERHOST ":[<reply>{<space><reply>}]"

                - Reply format used by USERHOST to list replies to
                  the query list.  The reply string is composed as
                  follows:

                  <reply> ::= <nick>['*'] '=' <'+'|'-'><hostname>

                  The '*' indicates whether the client has registered
                  as an Operator.  The '-' or '+' characters represent
                  whether the client has set an AWAY message or not
                  respectively.

303     RPL_ISON ":[<nick> {<space><nick>}]"

                - Reply format used by ISON to list replies to the
                  query list.

301     RPL_AWAY "<nick> :<away message>"
...

**Note: all the command replies is grouped in 200 and 300 range**

# TODO
1. Use of standard IRC command protocol to send command and receive command replies
	* Structure of the command
```
   The BNF representation for this is:

<message>  ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>

<prefix>   ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
<command>  ::= <letter> { <letter> } | <number> <number> <number>
<SPACE>    ::= ' ' { ' ' }
<params>   ::= <SPACE> [ ':' <trailing> | <middle> <params> ]

<middle>   ::= <Any *non-empty* sequence of octets not including SPACE
               or NUL or CR or LF, the first of which may not be ':'>
<trailing> ::= <Any, possibly *empty*, sequence of octets not including
                 NUL or CR or LF>

<crlf>     ::= CR LF
```
Note: :<prefix> is required in case of server communicating with other server and is not required when a client sends <message> to server because server knows from where the <message> is coming from because it's a TCP connection.

2. Create a request struct that will hold the request information (message) sent by client to server. request struct will provide easy access to different parts of message.

3. And also create a response struct that will hold the response informatin (message) sent by server to client. response struct will provide easy access to different parts of message.

4. Create functions like chat\_handle\_request() and chat\_prepare\_requst(). First function will read the message in the buffer and second function will take that buffer and parse it properly and then return prepared request structure.

5. Create functions like chat\_handle\_response() and chat\_prepare\_response(). First function will read the message in the buffer and second function will take that buffer and parse it properly and then return prepared response structure.

6. Create some function for client to handle preperation of request. These function will simplify the preperation of request and correctly prepare request struct that is further used by other function to send the request to server.

7. Same for the server. Create some function for server to handle preperation of response just like request in point number 6.

8. All of theses new functions will be prefixed bith `chat_` namespace.

9. Unify the request and response message.

10. Handle any error case in uniform way

11. All the Error and Reply message from server should comply with IRC protocol as specified in RFC standard

12. In the client program client should not be blocked in the read function call. Instead as soon as client connects to server, represent the client with no nick prompt and user will give command to create his nick and connect to any user, list all online users, connect to any user etc. all of this using command. All the client application commands are listed below.

13. Client application should not be blocked in a read function call on stdin or socket.

14. Change the `pair_fd` member of `struct client` structure to some kind of connected to member because in IRC it allows user to send message to a channel or to a specific user. Since client sends message with PRIVMSG command with nick, to whom the message destined to, server can't store file descriptior for a channel because it make sense.

15. Allow user to send message to another user or channel to whom the client is currently connected to.
* Server sends fatal error if an invalid command is sent by client


16. Implement `nick` command 					-- done
17. Implement `names` command					-- done
18. Implement `join` command					-- done
19. Implement `msg` command						-- done
20. Implement `quit` command


# Message commands for all requests
* Set nick name
* Show all online nick names including the client him self
* Assocciate with user or channel
* Send message (connected user will receive the message) - only paired associated users can exchange the message with each other (one-one pair)
* Disassociate user with each other (after disassociation user can no longer send message to each other, they need to re-associate to exchange messages)
* Quit user - user quits and the client application is closed

## Set nick name
   Command: NICK
Parameters: <nickname> [ <hopcount> ]

Example
```
NICK Wiz                        ; Introducing new nick "Wiz".

:WiZ NICK Kilroy                ; WiZ changed his nickname to Kilroy.
```
Numeric Replies:
```
ERR_NONICKNAMEGIVEN             ERR_ERRONEUSNICKNAME
ERR_NICKNAMEINUSE               ERR_NICKCOLLISION
```

Example replies
```
431 ERR_NONICKNAMEGIVEN :No nickname given
432 ERR_ERRONEUSNICKNAME <nick> :Erroneus nickname
433 ERR_NICKNAMEINUSE <nick> :Nickname is already in use

436 ERR_NICKCOLLISION <nick> :Nickname collision KILL       -- I won't implement it

No response is sent in case of successful nick assignment but I'll use not used code 300 for command response https://tools.ietf.org/html/rfc1459#section-6.2

300 RPL_NONE <nick> :Nickname changed successfully
```

User command
```
/nick <new_nick_name>
/nick <current_nick_name> <new_nick_name> 		; to change nick
```

## Show the nicks of all users on channel #channel
For my implementation for now, I will show all the nicks in the currently connected servers
   Command: NAMES
Parameters: [<channel>{,<channel>}]

Example
```
NAMES #twilight_zone,#42        ; list visible users on #twilight_zone
                                   and #42 if the channels are visible to
                                   you.

NAMES                           ; list all visible channels and users
```

Example replies
```
353 RPL_NAMREPLY <channel> :[[@|+]<nick> [[@|+]<nick> [...]]]
								; nicks are seperated by space
								; I will not implement channel version yet
366 RPL_ENDOFNAMES <channel> :End of /NAMES list
								; To reply to a NAMES message, a reply pair consisting
                  				; of RPL_NAMREPLY and RPL_ENDOFNAMES is sent by the
                  				; server back to the client. If there is no channel
                  				; found as in the query, then only RPL_ENDOFNAMES is
                  				; returned.  The exception to this is when a NAMES
                  				; message is sent with no parameters and all visible
                  				; channels and contents are sent back in a series of
                  				; RPL_NAMEREPLY messages with a RPL_ENDOFNAMES to mark
								; the end
```

User command
```
/names							; shows all nicks
/names #chanel1					; shows all nicks of channel #channel1
/names #chanel1[ #channel]		; general form of command /names
```

## Assocciate with a user for chatting (not IRC standard it is my implementation)
This will allow a user to connect with a specific user and exchange messages. This will allow user to send message without having to write any extra command. User will just write there message in client and presses enter
User will always send a private message to a specific user using IRC's `/msg <nick> <message goes here>` command

Note: This command is not supported by RFC-1459. It is non standard command. This is my custom command to support this feature for my version of chat app

   Command: ASSOC
Parameters: <nick>

Example
```
ASSOC Wiz 						; connected client reqquest to associate with Wiz
:Abc ASSOC Wiz 					; Abc requested to associate with Wiz
```
Numeric replies
```
ERR_NORECIPIENT
ERR_NOSUCHNICK
```

Example replies
```
411 ERR_NORECIPIENT :No recipient given (<command>)
401 ERR_NOSUCHNICK <nickname> :No such nick/channel

other replies will be implemented in future, I don't need it for now
```
User command
```
/join <nick>					; user can only be associated one user or channel at any given time
								; join command is used to connect to user and channel. If parameter
								; starts with '#' (hash) then client want to connect to channel else
								; user
```

## Send message 
   Command: PRIVMSG
Parameters: <receiver>{,<receiver>} <text to be sent>

Example
```
:Angel PRIVMSG Wiz :Hello are you receiving this message ?
                                ; Message from Angel to Wiz.
PRIVMSG Wiz :yes I'm receiving it!
								; message to Wiz
```

Numeric replies
```
ERR_NORECIPIENT                 ERR_NOTEXTTOSEND
ERR_CANNOTSENDTOCHAN            ERR_NOTOPLEVEL
ERR_WILDTOPLEVEL                ERR_TOOMANYTARGETS
ERR_NOSUCHNICK
RPL_AWAY

```

Example replies
```
411 ERR_NORECIPIENT :No recipient given (<command>)
412 ERR_NOTEXTTOSEND :No text to send
401 ERR_NOSUCHNICK <nickname> :No such nick/channel
301 RPL_AWAY <nick> :<away message>

other replies will be implemented in future, I don't need it for now
```

User command
```
just type your message			; message to be sent to associated user
/msg <nick> <message>			; message to nick with message
```

## Disassociate user with each other
PART command tells server to disconnect connected user from <receiver>
   Command: PART
Parameters: <receiver>
<receiver> can be nick or channel, but I am implementing nick for now

Example
```
PART Wiz 						; disconnect reqquesting client from Wiz
:Abc ASSOC Wiz 					; Abc requested to disconnect from Wiz
```
Numeric replies
```
ERR_NORECIPIENT
ERR_NOSUCHNICK
ERR_NOTONCHANNEL
```

Example replies
```
411 ERR_NORECIPIENT :No recipient given (<command>)
401 ERR_NOSUCHNICK <nickname> :No such nick/channel
442 ERR_NOTONCHANNEL <channel> :You're not on that channel

other replies will be implemented in future, I don't need it for now
```
User command
```
/part <receiver>				; disassociate from given receiver (nick or channel)
```
## Quit user
   Command: QUIT
Parameters: [<Quit message>]
A client session is ended with a quit message.

Example
```
QUIT :Gone to have lunch        ; Preferred message format.
```
Numeric replies
```
None
```

User command
```
/quit [message]
```

**Note:** All the Errors ERR\_ and Replies RPL\_ is only sent by server in response to user's command and when a user sends message to another user then server does not relay this message to user at the other end as RPL\_ instead structure of message becomes
1. Client sends messge
```
PRIVMSG bob :Hey Bob!
```
2. Server receives it and re-structures the message as
```
:bob!hostname@bobserver PRIVMSG bob :Hey Bob!
```
then sends it to other end (to the user)

Handle this situation correctly in client side. Because client can get response from server as followings:
* When server replies with error in response to client's command
```
:serverhost <errorcode> <nick> <error message according to error code as defined in RFC-1459>
```
serverhost is the server host name replying this message
errorcode  is the code to specify the error as mentioned in RFC-1459
nick       is the nick of user for whom this error message is destined for
error msg  is the message that describes the error. Description for each error is mentioned in the RFC-1459 and must be used for any error description

* When server replies with RPL in response to client's command
This kind of replies is used when server sends some info to requesting client like when client query for online user's nick and channels etc.
```
:serverhost <rplcode> <nick> <reply message according to reply code as defined in RFC-1459>
```
serverhost is the server host name replying this message
rplcode    is the code to specify the reply as mentioned in RFC-1459
nick       is the nick of user for whom this reply message is destined for
reply msg  is the content that server sends to client

* When server sends message of one user to another user
This kind of message passing is done when user sends PRIVMSG message to another user or sends message to a channel
```
:serverhost <rplcode> <nick> <reply message according to reply code as defined in RFC-1459>
```
serverhost is the server host name replying this message
rplcode    is the code to specify the reply as mentioned in RFC-1459
nick       is the nick of user for whom this reply message is destined for
reply msg  is the content that server sends to client
```
:bob!hostname@bobserver PRIVMSG bob :Hey Bob!
```
!hostname@bobserver is not required just the nick name is enough
