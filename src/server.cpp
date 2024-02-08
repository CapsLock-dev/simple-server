#include <server.hpp>
#include <sys/epoll.h>

Server::Server(int port) {
	int err = 0;
	int opt = 1;
	sockaddr_in addr;
	// Create socket
	m_socketfd = socket(AF_INET, SOCK_STREAM, 0);
	fcntl64(m_socketfd, F_SETFL, O_NONBLOCK); // Enabling non-blocking
	if (m_socketfd == -1) {
		perror("socket error");
		exit(1);
	}
	// Enable reusing address and port
	err = setsockopt(m_socketfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)); 
	if (err != 0) {
		perror("setsockopt error");
		exit(1);
	}
	// Bind socket to port
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	err = bind(m_socketfd, (sockaddr*)&addr, sizeof(addr));
	if (err != 0) {
		perror("bind error");
		exit(1);
	}
	std::cout << "Socket started\n";
}

void Server::listen() {
	int max_events = 20;
	epoll_event ev, events[max_events];
	int err = 0;
	int epoll_fd = epoll_create1(0);
	if (epoll_fd == -1) {	
		perror("epoll_create error");
		exit(1);
	}
	ev.events = EPOLLIN | EPOLLET;
	err = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, m_socketfd, &ev);
	if (err != 0) {
		perror("epoll_ctl error");
		exit(1);
	}
	std::cout << "Waiting for connections...\n";
	while (true) {
		int ready_sockets = epoll_wait(epoll_fd, events, max_events, 10);
		if (ready_sockets == -1) {
			perror("epoll_wait error");
			exit(1);
		}
		for (int i=0; i<ready_sockets; i++) {
			int sock = events[i].data.fd;
			// if sock is server socket - handle connection
			if (sock == m_socketfd) {

			}
		}
	}
}

Server::~Server() {
	if (m_socketfd != -1) {
		close(m_socketfd);
		std::cout << "Socket closed\n";
	}	
}
