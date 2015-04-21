
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
#include <string>
#include <algorithm>
using namespace std;

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
    while (commandLine[i] != ' ' && commandLine[i] != '\n' && commandLine[i] != '\r') {
        password[j] = commandLine[i];
        i++;
        j++;
    }
    if (commandLine[i] == ' ') {
        password[j] = '\0';
        j = 0;
        i++;
        args[j] = ' ';

        while (commandLine[i] != ' ' && commandLine[i] != '\r') {
            args[j] = commandLine[i];
            i++;
            j++;
        }
        if (commandLine[i] == ' ') { 
            args[j] = '\0';
            j = 0;
            i++;
            while (commandLine[i] != '\0' && commandLine[i] != '\n') {
                message[j] = commandLine[i];
                i++;
                j++;
            }
            message[j] = '\0';
        } else {
            message[0] = '\0';
        }
    } else {
        message[0] = '\0';
        args[0] = '\0';
    }

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
        getMessages(fd, user, password, args, message);
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
    // Initialize users in room
    referenceRoom = NULL;
    // Initalize message list

}

bool
IRCServer::checkPassword(int fd, const char * user, const char * password) {
    // Here check the password

    char name[50], passworded[50], holder[100];
    FILE * file = fopen("password.txt", "r");

    while (fgets(holder, 100, file)) {
        sscanf (holder, "%s %s \n", name, passworded);
        if (!strcmp(name, user) && !strcmp(password, passworded)) {
            fclose(file);
            return true;
        }
    }
    const char * msg = "ERROR (Wrong password)\r\n";
    write (fd, msg, strlen(msg));
    fclose(file);
    return false;
}

    void
IRCServer::addUser(int fd, const char * user, const char * password, const char * args)
{

    // Here add a new user. For now always return OK.
    char holder[100], name[50], passworded[50];

    FILE * file = fopen("password.txt", "a+");
    while (fgets(holder, 100, file)) {
        sscanf (holder, "%s %s\n", name, passworded);
        if (!strcmp(name, user)) {
            const char * rsp = "DENIED\r\n";
            write (fd, rsp, strlen(rsp));
            fclose(file);
            return;
        }
        holder[0] = '\0';
        name[0] = '\0';
        passworded[0] = '\0';
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

    for (int i = 0; i < 100; i++) { 
        newRoom->Message[i] = NULL;

    }

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
            const char * msg = "DENIED\n";
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

    Room * r = referenceRoom;

    // Here add a new user. For now always return OK.


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

            const char * msg = "OK\r\n";
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

    while (strcmp(args, r->roomName)) {
        r = r->nextRoom;
    }
    Chatter * n = r->inRoom;
    Chatter * prev = NULL;

    while (n != NULL) {
        if (!strcmp(user,n->name)) {
            if (prev == NULL) {
                prev = n->next;
                r->inRoom = prev;
            } else {
                prev->next = n->next;
            }
            const char * msg = "OK\r\n";
            write (fd, msg, strlen(msg));
            return;
        }
        prev = n;
        n = n->next;
    }
    const char * msg = "ERROR (No user in room)\r\n";
    write (fd, msg, strlen(msg));
}
int checked = 1;
    void
IRCServer::sendMessage(int fd, const char * user, const char * password, const char * args, const char * message)
{
    const char * msg = "OK\r\n";
    if (checkPassword(fd, user, password) == false) {
        return;
    }
    Room * r = referenceRoom;
    while (strcmp(args, r->roomName)) {
        r = r->nextRoom;
    }
    
    Chatter * n = r->inRoom;
    int checked = 0;
    while (n != NULL) {
        if (!strcmp(n->name, user)) {
            checked = 1;
        }
        n = n->next;
    }

if (checked == 1) {
    if (r->msgnum == 100) {
        for (int i = 0; i < 98; i ++) {
            r->Message[i] = r->Message[i+1];
            r->sender[i] = strdup(user);
        }
        r->Message[99] = strdup(message);
        r->sender[99] = strdup(user);
        write (fd, msg, strlen(msg));
        return;
    }
    r->Message[r->msgnum] = strdup(message);
    r->sender[r->msgnum] = strdup(user);
    r->msgnum++;
    write (fd, msg, strlen(msg));
} else {
    const char * rsp = "ERROR (user not in room)\r\n";
    write (fd, rsp, strlen(rsp));
}
}

    void
IRCServer::getMessages(int fd, const char * user, const char * password, char * args, const char * message)
{
    if (checkPassword(fd, user, password) == false) {
        return;
    }
    Room * r = referenceRoom;
    while (strcmp(message, r->roomName)){
        r = r->nextRoom;
    }

    Chatter * n = r->inRoom;
    int checked = 0;
    while (n != NULL) {
        if (!strcmp(n->name, user)) {
            checked = 1;
        }
        n = n->next;
    }

    if (checked == 1) {
    int i = (int) *args - '0';

    if (r->Message[i] == NULL) {
        const char * wordz = "NO-NEW-MESSAGES\r\n";
        write (fd, wordz, strlen(wordz));
        return;
    }
    for (; i < 99; i++) {
        if (r->Message[i] == NULL ) {
            const char * newLine = "\r\n";
            write (fd, newLine, strlen(newLine));
            return;
        }
        char num[50];
        sprintf(num, "%d", i);
        const char * msg = strdup(r->Message[i]);
        const char * usr = strdup(r->sender[i]);
        const char * space = " ";
        const char * newLine = "\r\n";
        write (fd, num, strlen(num));
        write (fd, space, strlen(space));
        write (fd, usr, strlen(usr));
        write (fd, space, strlen(space));
        write (fd, msg, strlen(msg));
        write (fd, newLine, strlen(newLine));
    }
    return;
    } else {
    const char * rsp = "ERROR (User not in room)\r\n";
    write (fd, rsp, strlen(rsp));
    return; 
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
    
    char NameHolder[50][50];
    int i = 0;

    while (n != NULL) {
        char * name = strdup(n->name);
        strcpy(NameHolder[i], name);
        i++;
        n = n->next;
    }
    i--;
    char temp[50];

    for (int a = 0; a < i; a++) {
        if (NameHolder[a][0] == NameHolder[a+1][0]) {
            for (int b = 0; b < 3; b++) {
                if (NameHolder[a][b] > NameHolder[a+1][b]) {
                    sscanf(NameHolder[a], "%s\n", temp);
                    sscanf(NameHolder[a+1], "%s\n", NameHolder[a]);
                    sscanf(temp, "%s\n", NameHolder[a+1]);
                    a = -1;
                    break;
                }
            }
        }
        if (NameHolder[a][0] > NameHolder[a+1][0]) {
            sscanf(NameHolder[a], "%s\n", temp);
            sscanf(NameHolder[a+1], "%s\n", NameHolder[a]);
            sscanf(temp, "%s\n", NameHolder[a+1]);
            a = -1;
        }
    }
    for (int a = 0; a < i+1; a++) {
        const char * newline = "\r\n";
        write (fd, NameHolder[a], strlen(NameHolder[a]));
        write (fd, newline, strlen(newline));
    }
    const char * newline = "\r\n";
    write (fd, newline, strlen(newline));
}

    void
IRCServer::getAllUsers(int fd, const char * user, const char * password,const  char * args)
{
    FILE * file = fopen("password.txt", "r");
    char holder[100];
    char name [50];
    char NameHolder[50][50];
    int i = 0;
    if (checkPassword(fd, user, password) == false) {
        return;
    }

    while (fgets(holder, 100, file)) {
        sscanf (holder, "%s\n", NameHolder[i]);
        i++;
    }
    i--;
    char temp[50];
    for (int a = 0; a < i; a++) {
        if (NameHolder[a][0] == NameHolder[a+1][0]) {
            for (int b = 0; b < 3; b++) {
                if (NameHolder[a][b] > NameHolder[a+1][b]) {
                    sscanf(NameHolder[a], "%s\n", temp);
                    sscanf(NameHolder[a+1], "%s\n", NameHolder[a]);
                    sscanf(temp, "%s\n", NameHolder[a+1]);
                    a = -1;
                    break;
                }
            }
        }
        if (NameHolder[a][0] > NameHolder[a+1][0]) {
            sscanf(NameHolder[a], "%s\n", temp);
            sscanf(NameHolder[a+1], "%s\n", NameHolder[a]);
            sscanf(temp, "%s\n", NameHolder[a+1]);
            a = -1;
        }
    }


    for (int a = 0; a < i+1; a++) {
        const char * newline = "\r\n";
        write (fd, NameHolder[a], strlen(NameHolder[a]));
        write (fd, newline, strlen(newline));
    }
    const char * newline = "\r\n";
    write (fd, newline, strlen(newline));
    fclose(file);
}



