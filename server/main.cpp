#include "TcpSocket.hpp"
#include "MessageTransport.hpp"
#include <cstdio>
#include <unistd.h>

void server() {
	TcpSocket sock;
	sock.bind("127.0.0.1", 4444);
	sock.listen(1);
	printf("Listening on 127.0.0.1:4444\n");
	TcpSocket conn = sock.accept();
	printf("Connection established\n");
	MessageTransport transport(conn);
	Message msg = {
		.id = 1,
		.type = Open,
		.length = 5,
	};
	msg.data.reserve(5);
	msg.data[0] = 'h';
	msg.data[1] = 'e';
	msg.data[2] = 'l';
	msg.data[3] = 'l';
	msg.data[4] = 'o';
	printf("Sent Message\n");
	transport.write(msg);
}

int main(int argc, char const* argv[])
{
	try {
		server();
		return 0;
	} catch (std::exception& e) {
		printf("Exception: %s\n", e.what());
	}
}
