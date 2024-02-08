#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>


class Server {
public:
	Server(int port);
	~Server();
	void listen();
private:
	int m_socketfd;
	void handle_connection();
	void handle_message();
};
