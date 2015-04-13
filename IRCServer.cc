
const char * usage =
"                                                               \n"
"IRCServer:                                                   \n"
"                                                               \n"
"Simple server program used to communicate multiple users       \n"
"                                                               \n"
"To use it in one window type:                                  \n"
"                                                               \n"
"   IRCServer <port>                                          \n"
"                                                               \n"
"Where 1024 < port < 65536.                                     \n"
"                                                               \n"
"In another window type:                                        \n"
"                                                               \n"
"   telnet <host> <port>                                        \n"
"                                                               \n"
"where <host> is the name of the machine where talk-server      \n"
"is running. <port> is the port number you used when you run    \n"
"daytime-server.                                                \n"
"                                                               \n";

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "IRCServer.h"

int QueueLength = 5;

typedef struct Chatter{
    char * name;
    Chatter * next;
} Chatter;


struct Room 
{
    char * roomName;
    int msgnum;
    Chatter * inRoom;
    Room * nextRoom;
    char * Message[100];
    char * sender[100];
};

typedef struct Room Room;
Room * referenceRoom;

int
IRCServer::open_server_socket(int port) {

    // Set the IP address and port for this server
    struct sockaddr_in serverIPAddress; 
    memset( &serverIPAddress, 0, sizeof(serverIPAddress) );
    serverIPAddress.sin_family = AF_INET;
    serverIPAddress.sin_addr.s_addr = INADDR_ANY;
    serverIPAddress.sin_port = htons((u_short) port);

    // Allocate a socket
    int masterSocket =  socket(PF_INET, SOCK_STREAM, 0);
    if ( masterSocket < 0) {
        perror("socket");
        exit( -1 );
    }

    // Set socket options to reuse port. Otherwise we will
    // have to wait about 2 minutes before reusing the sae port number
    int optval = 1; 
    int err = setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR, 
            (char *) &optval, sizeof( int ) );

    // Bind the socket to the IP address and port
    int error = bind( masterSocket,
            (struct sockaddr *)&serverIPAddress,
            sizeof(serverIPAddress) );
    if ( error ) {
        perror("bind");
        exit( -1 );
    }

    // Put socket in listening mode and set the 
    // size of the queue of unprocessed connections
    error = listen( masterSocket, QueueLength);
    if ( error ) {
        perror("listen");
        exit( -1 );
    }

    return masterSocket;
}

    void
IRCServer::runServer(int port)
{
    int masterSocket = open_server_socket(port);

    initialize();

    while ( 1 ) {

        // Accept incoming connections
        struct sockaddr_in clientIPAddress;
        int alen = sizeof( clientIPAddress );
        int slaveSocket = accept( masterSocket,
                (struct sockaddr *)&clientIPAddress,
                (socklen_t*)&alen);

        if ( slaveSocket < 0 ) {
            perror( "accept" );
            exit( -1 );
        }

        // Process request.
        processRequest( slaveSocket );		
    }
}

    int
main( int argc, char ** argv )
{
    // Print usage if not enough arguments
    if ( argc < 2 ) {
        fprintf( stderr, "%s", usage );
        exit( -1 );
    }

    // Get the port from the arguments
    int port = atoi( argv[1] );

    IRCServer ircServer;

    // It will never return
    ircServer.runServer(port);

}

//
// Commands:
//   Commands are started y the client.
//
//   Request: ADD-USER <USER> <PASSWD>\r\n
//   Answer: OK\r\n or DENIED\r\n
//
//   REQUEST: GET-ALL-USERS <USER> <PASSWD>\r\n
//   Answer: USER1\r\n
//            USER2\r\n
//            ...
//            \r\n
//
//   REQUEST: CREATE-ROOM <USER> <PASSWD> <ROOM>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: LIST-ROOMS <USER> <PASSWD>\r\n
//   Answer: room1\r\n
//           room2\r\n
//           ...
//           \r\n
//
//   Request: ENTER-ROOM <USER> <PASSWD> <ROOM>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: LEAVE-ROOM <USER> <PASSWD>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: SEND-MESSAGE <USER> <PASSWD> <MESSAGE> <ROOM>\n
//   Answer: OK\n or DENIED\n
//
//   Request: GET-MESSAGES <USER> <PASSWD> <LAST-MESSAGE-NUM> <ROOM>\r\n
//   Answer: MSGNUM1 USER1 MESSAGE1\r\n
//           MSGNUM2 USER2 MESSAGE2\r\n
//           MSGNUM3 USER2 MESSAGE2\r\n
//           ...\r\n
//           \r\n
//
//    REQUEST: GET-USERS-IN-ROOM <USER> <PASSWD> <ROOM>\r\n
//    Answer: USER1\r\n
//            USER2\r\n
//            ...
//            \r\n
//

    void
IRCServer::processRequest( int fd )
{
    // Buffer used to store the comand received from the client
    const int MaxCommandLine = 1024;
    char commandLine[ MaxCommandLine + 1 ];
    int commandLineLength = 0;
    int n;

    // Currently character read
    unsigned char prevChar = 0;
    unsigned char newChar = 0;

    //
    // The client should send COMMAND-LINE\n
    // Read the name of the client character by character until a
    // \n is found.
    //

    // Read character by character until a \n is found or the command string is full.
    while ( commandLineLength < MaxCommandLine &&
            read( fd, &newChar, 1) > 0 ) {

        if (newChar == '\n' && prevChar == '\r') {
            break;
        }

        commandLine[ commandLineLength ] = newChar;
        commandLineLength++;

        prevChar = newChar;
    }

    // Add null character at the end of the string
    // Eliminate last \r
    commandLineLength--;
    commandLine[ commandLineLength ] = '\0';

    printf("RECEIVED: %s\n", commandLine);

    int i = 0, j = 0;
    char command[50];
    char user[50];
    char password[50];
    char args[50];
    char message[1000];

    while (commandLine[i] != ' ') {
        command[i] = commandLine[i];
        i++;
    }
    command[i] = '\0';
    i++;
    while (commandLine[i] != ' ') {
        user[j] = commandLine[i];
        i++;
        j++;
    }
    user[j] = '\0';
    j = 0;
    i++;
    while (commandLine[i] != ' ') {
        password[j] = commandLine[i];
        i++;
        j++;
    }
    password[j] = '\0';
    j = 0;
    i++;
    args[j] = ' ';
    while (commandLine[i] != ' ') {
        args[j] = commandLine[i];
        i++;
        j++;
    }
    args[j] = '\0';
    j = 0;
    i++;
    while (commandLine[i] != '\0') {
        message[j] = commandLine[i];
        i++;
        j++;
    }
    message[j] = '\0';


    printf("command=%s\n", command);
    printf("user=%s\n", user);
    printf( "password=%s\n", password );
    printf("args=%s\n", args);
    printf("message=%s\n", message);
    
    if (!strcmp(command, "ADD-USER")) {
        addUser(fd, user, password, args);
    }
    else if (!strcmp(command, "ENTER-ROOM")) {
        enterRoom(fd, user, password, args);
    }
    else if (!strcmp(command, "LEAVE-ROOM")) {
        leaveRoom(fd, user, password, args);
    }
    else if (!strcmp(command, "SEND-MESSAGE")) {
        sendMessage(fd, user, password, args, message);
    }
    else if (!strcmp(command, "GET-MESSAGES")) {
        getMessages(fd, user, password, args);
    }
    else if (!strcmp(command, "GET-USERS-IN-ROOM")) {
        getUsersInRoom(fd, user, password, args);
    }
    else if (!strcmp(command, "GET-ALL-USERS")) {
        getAllUsers(fd, user, password, args);
    }
    else if (!strcmp(command, "CREATE-ROOM")) {
        createRoom(fd, user, password, args);
    }
    else {
        const char * msg =  "UNKNOWN COMMAND\r\n";
        write(fd, msg, strlen(msg));
    }

    // Send OK answer
    //const char * msg =  "OK\n";
    //write(fd, msg, strlen(msg));

    close(fd);	
}


    void
IRCServer::initialize()
{
    // Open password file
    fopen("passwords.txt", "a+");
    // Initialize users in room
    referenceRoom = NULL;

    // Initalize message list

}

bool
IRCServer::checkPassword(int fd, const char * user, const char * password) {
    // Here check the password

    char name[50], passworded[50], holder[100];
    FILE * file = fopen("passwords.txt", "r");

    while (fgets(holder, 100, file)) {
        sscanf (holder, "%s %s \n", name, passworded);
        if (!strcmp(name, user) && !strcmp(password, passworded)) {
            return true;
        }
    }
    const char * msg = "Incorrect password\n";
    write (fd, msg, strlen(msg));
    return false;
}

    void
IRCServer::addUser(int fd, const char * user, const char * password, const char * args)
{

    FILE * file = fopen("passwords.txt", "a+");
    // Here add a new user. For now always return OK.
    char holder[100], name[50];

    while (fgets(holder, 100, file)) {
        sscanf (holder, "%s\n", name);
        if (!strcmp(name, user)) {
            const char * rsp = "Name already taken\r\n";
            write (fd, rsp, strlen(rsp));
            fclose(file);
            return;
        }
    }

    fprintf(file, "%s %s\n", user, password);
    const char * msg =  "OK\r\n";
    write(fd, msg, strlen(msg));
    fclose(file);
    return;		
}
    void
IRCServer::createRoom (int fd, const char * user, const char * password, const char * args) 
{
    if (!checkPassword(fd, user, password)){
        return;
    }
    Room * r = referenceRoom;
    Room * newRoom = (Room *) malloc(sizeof(Room));
    char holder[100], name[50];
    newRoom->msgnum = 0;


    Chatter * n = (Chatter *) malloc(sizeof(Chatter));


    if (referenceRoom == NULL) {
        const char * msg =  "OK\r\n";

        newRoom->roomName = strdup(args);
        write(fd, msg, strlen(msg));
        referenceRoom = newRoom;
        return;
    }
    while (r != NULL) { 
        if (!strcmp(args, r->roomName)) {
            const char * msg = "Room already exists\n";
            write (fd, msg, strlen(msg));
            return;
        }
        r = r->nextRoom;
    }

    const char * msg =  "OK\r\n";
    write(fd, msg, strlen(msg));
    r = referenceRoom;
    while (r->nextRoom != NULL) {
        r = r->nextRoom;
    }

    r->nextRoom = newRoom;
    newRoom->roomName = strdup(args);
    return;
}
    void
IRCServer::enterRoom(int fd, const char * user, const char * password, const char * args)
{
    if (!checkPassword(fd, user, password)) {
        return;
    }

    Room * newRoom = (Room *) malloc(sizeof(Room));
    char holder[100], name[50];

    Chatter * n = (Chatter *) malloc(sizeof(Chatter));
    n->name = strdup(user);
    n->next = NULL;



    // Here add a new user. For now always return OK.
    Room * r = referenceRoom;

    Chatter * it = r->inRoom;
    while (r != NULL) {
        if (!strcmp(args, r->roomName)) {
            if (r->inRoom == NULL) {
                r->inRoom = n;

            } else {
                it = r->inRoom;
                while (it->next != NULL) {
                    it = it->next;
                }
                it->next = n;
            }

            const char * msg = "You joined the room!\n";
            write (fd, msg, strlen(msg));
            return;
        }

        r = r->nextRoom;
    }



}

    void
IRCServer::leaveRoom(int fd, const char * user, const char * password, const char * args)
{
    if (checkPassword(fd, user, password) == false) {
        return;
    }
    Room * r = referenceRoom;

    while (strcmp(args, r->roomName) == 1) {
        r = r->nextRoom;
    }
    Chatter * n = r->inRoom;
    Chatter * prev = NULL;

    while (n != NULL) {
        if (!strcmp(user,n->name)) {
            if (prev == NULL) {
                prev->next = n->next;
                r->inRoom = prev;
            } else {
                prev->next = n->next;
            }
            const char * msg = "You have left the room!\n";
            write (fd, msg, strlen(msg));
        }
        prev = n;
        n = n->next;
    }
}

    void
IRCServer::sendMessage(int fd, const char * user, const char * password, const char * args, const char * message)
{
    if (checkPassword(fd, user, password) == false) {
        return;
    }
    Room * r = referenceRoom;
    while (strcmp(args, r->roomName) == 1) {
        r = r->nextRoom;
    }

    if (r->msgnum == 99) {
        for (int i = 0; i < 99; i ++) {
                r->Message[i] = r->Message[i+1];
                r->sender[i] = strdup(user);
        }
    }
        r->Message[r->msgnum] = strdup(message);
        r->sender[r->msgnum] = strdup(user);
    if (r->msgnum == 99) {
        r->msgnum = -1;
    }
    r->msgnum++;
}

    void
IRCServer::getMessages(int fd, const char * user, const char * password, const char * args)
{
    if (checkPassword(fd, user, password) == false) {
        return;
    }
    Room * r = referenceRoom;
    while (strcmp(args, r->roomName) == 1) {
        r = r->nextRoom;
    }
    for (int i = 0; i < 99; i++) {
        if (r->Message[i] == NULL) {
            return;
        }
        const char * msg = strdup(r->Message[i]);
        const char * usr = strdup(r->sender[i]);
        const char * semi = ": ";
        const char * newLine = " \n";
        write (fd, usr, strlen(usr));
        write (fd, semi, strlen(semi));
        write (fd, msg, strlen(msg));
        write (fd, newLine, strlen(newLine));
    }
}

    void
IRCServer::getUsersInRoom(int fd, const char * user, const char * password, const char * args)
{
    if (checkPassword(fd, user, password) == false) {
        return;
    }
    Room * r = referenceRoom;
    while (strcmp(args, r->roomName) == 1) {
        r = r->nextRoom;
    }
    Chatter * n = r->inRoom;

    if (n == NULL) {
        const char * msg = "The room is empty!\n";
        write(fd, msg, strlen(msg));
        return;
    }

    const char * newline = " \n";
    while (n != NULL) {
        char * name = strdup(n->name);
        write (fd, name, strlen(name));
        n = n->next;
        write (fd, newline, strlen(newline));
    }
}

    void
IRCServer::getAllUsers(int fd, const char * user, const char * password,const  char * args)
{
    FILE * file = fopen("passwords.txt", "r");
    char holder[100], name[50];

    if (checkPassword(fd, user, password) == false) {
        return;
    }

    while (fgets(holder, 100, file)) {
        sscanf (holder, "%s\n", name);
        const char * newline = " \n";
        write (fd, name, strlen(name));
        write (fd, newline, strlen(newline));
    }
    fclose(file);
}

