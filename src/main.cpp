#include <iostream>
#include "server.hpp"

int main(int argc, char** argv)
{
	Server server(1922);
	server.listen_conn();
	return 0;
}
