#include <arpa/inet.h>
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <cerrno>
#include <server.hpp>
#include <sys/epoll.h>
#include <sys/socket.h>

Server::Server(int32_t port) {
	int32_t err = 0;
	int32_t opt = 1;
	sockaddr_in addr;
	// Create socket
	m_serverfd = socket(AF_INET, SOCK_STREAM, 0);
	fcntl64(m_serverfd, F_SETFL, O_NONBLOCK); // Enabling non-blocking
	if (m_serverfd == -1) {
		perror("socket error");
		exit(1);
	}
	// Enable reusing address and port
	err = setsockopt(m_serverfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)); 
	if (err != 0) {
		perror("setsockopt error");
		exit(1);
	}
	// Bind socket to port
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	err = bind(m_serverfd, (sockaddr*)&addr, sizeof(addr));
	if (err != 0) {
		perror("bind error");
		exit(1);
	}
	std::cout << "Socket started\n";
	// Create epoll	
	m_epollfd = epoll_create1(0);
	if (m_epollfd == -1) {
		perror("epoll_create error");
		exit(1);
	}
	err = add_epoll(m_serverfd, EPOLLIN | EPOLLET);
	if (!err) {
		perror("add_epoll error");
		exit(1);
	}
}

void Server::listen_conn() {
	int32_t err = 0;

	err = listen(m_serverfd, m_backlog);
	if (err != 0) {
		perror("listen errror");
		exit(1);
	}
	
	std::cout << "Listening for connections on all ips...\n";
	int32_t ready_fds = 0;
	int32_t max_events = 30;
	epoll_event events[max_events];
	while (true) {
		ready_fds = epoll_wait(m_epollfd, events, max_events, 1);
		if (ready_fds == -1) {
			perror("epoll_wait error");
			exit(1);
		}
		for (int32_t i=0; i < ready_fds; i++) {
			std::clog << "Event: " << events[i].events << "\n";
			if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLRDHUP) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN))) {
				close_connection(events[i].data.fd);
			} else if (events[i].data.fd == m_serverfd) {
				handle_connection();
			} else if (events[i].events & EPOLLIN){	
				handle_message(events[i].data.fd);
			} else {
				std::cout << "Wrong event\n";
				close_connection(events[i].data.fd);
			}
		}
	}
}

void Server::handle_connection() {
	// accept connections 
	struct sockaddr addr;
	socklen_t addr_len = sizeof(addr);
	int32_t client = accept(m_serverfd, &addr, &addr_len);
	if (client == -1) {
		std::cout << "Failed to accept connection\n";
		return;
	}
	fcntl64(client, F_SETFL, O_NONBLOCK); // Enabling non-blocking
	add_epoll(client, EPOLLIN | EPOLLET | EPOLLERR | EPOLLRDHUP | EPOLLHUP);
	char addr_str[INET_ADDRSTRLEN];	
	inet_ntop(AF_INET, &((sockaddr_in*)&addr)->sin_addr, addr_str, INET_ADDRSTRLEN);
	std::cout << "Got connection from" << addr_str << "\n";
}

void Server::handle_message(int32_t sock) {
	std::vector<char> msg = {};
	char buff[1024] = {};
	uint32_t buffsize = sizeof(buff);
	int32_t readsize = recv(sock, buff, buffsize, 0);
	while (readsize > 0) {
		readsize =  recv(sock, buff, buffsize, 0); 
		for (char c : buff) {
			msg.push_back(c);
		}
	}
	std::cout << "Recieved message: ";
	for (char c : msg) {
		std::cout << c;
	}
	std::cout << "\n";
	if (readsize == -1 && errno != EAGAIN) {	
		std::cout << "Error while reading. Closing connection " << errno << "\n";
		close(sock);
	}
}

bool Server::add_epoll(int32_t fd, int32_t event) {
	int32_t err = 0;
	epoll_event ev;
	ev.events = event;
	ev.data.fd = fd;
	err = epoll_ctl(m_epollfd, EPOLL_CTL_ADD, fd, &ev);
	if (err != 0) {
		perror("epoll_ctl error");
		return false;
	}
	return true;
}

bool Server::close_connection(int32_t fd) {	
	int32_t err = 0;
	sockaddr addr;
	socklen_t addr_size;
	getsockname(fd, &addr, &addr_size);
	char addr_str[INET_ADDRSTRLEN];	
	inet_ntop(AF_INET, &((sockaddr_in*)&addr)->sin_addr, addr_str, INET_ADDRSTRLEN);

	std::cout << "Closed connection with " <<  addr_str << "\n";
	err = close(fd);
	if (err != 0) {
		std::cerr << "Failed to close connection with " << addr_str; 
		return false;
	}
	return true;
}

Server::~Server() {
	if (m_serverfd != -1) {
		close(m_serverfd);
		std::cout << "Socket closed\n";
	}	
}
