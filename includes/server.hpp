#include <sys/socket.h>
#include <iostream>
#include <vector>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>


class Server {
public:
	Server(int32_t port);
	~Server();
	void listen_conn();
private:
	int32_t m_serverfd;
	int32_t m_epollfd;
	int32_t m_backlog;
	void handle_connection();
	void handle_message(int32_t fd);
	bool add_epoll(int32_t fd, int32_t ev);
	bool close_connection(int32_t fd);
};
