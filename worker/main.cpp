#include "TcpSocket.hpp"
#include "MessageTransport.hpp"
#include <cstdio>
#include <unistd.h>

void worker() {
	TcpSocket sock;
	sock.connect("127.0.0.1", 4444);
	MessageTransport transport(sock);
	Message msg = {0};
	transport.read(msg);
	printf("Got Message\n");
	printf("ID: %d\n", msg.id);
	printf("Type: %d\n", msg.type);
	printf("Length: %d\n", msg.length);
	printf("Data[0]: %c\n", msg.data[0]);
	printf("Data[4]: %c\n", msg.data[4]);
}

int main(int argc, char const* argv[])
{
	try {
		worker();
		return 0;
	} catch (std::exception& e) {
		printf("Exception: %s\n", e.what());
	}
}
