
#ifndef IRC_SERVER
#define IRC_SERVER

#define PASSWORD_FILE "password.txt"






class IRCServer {
	// Add any variables you need

public:
/*    typedef struct User{
        char * name;
        char * password;
        } User;

    typedef struct Chatter{
        char * name;
        Chatter * next;
    } Chatter;

    typedef struct Message{
        char * msg;
        char * snder;
        Message * next;
    } Message;

    typedef struct Room{
        char * roomName;
        Message * start;
        Chatter * inRoom;
        Room * nextRoom;
    } Room;
*/

private:
	int open_server_socket(int port);

public:
	void initialize();
	bool checkPassword(int fd, const char * user, const char * password);
	void processRequest( int socket );
	void addUser(int fd, const char * user, const char * password, const char * args);
	void enterRoom(int fd, const char * user, const char * password, const char * args);
	void leaveRoom(int fd, const char * user, const char * password, const char * args);
	void sendMessage(int fd, const char * user, const char * password, const char * args, const char * message);
	void getMessages(int fd, const char * user, const char * password, char * args, const char * message);
    void createRoom(int fd, const char * user, const char * password, const char * args);
    void getRooms (int fd, const char * user, const char * password, const char * args);
	void getUsersInRoom(int fd, const char * user, const char * password, const char * args);
	void getAllUsers(int fd, const char * user, const char * password, const char * args);
	void runServer(int port);
};

#endif
