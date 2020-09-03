#include "TcpSocket.hpp"
#include "MessageTransport.hpp"
#include "BinarySerializer.hpp"
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
	BinarySerializer serializer;
	Message msg = {
		.id = 1,
		.type = Open,
	};
	serializer.serialize_str("Hello World\n");
	msg.data = serializer.data();
	msg.length = msg.data.size();
	transport.write(msg);
	printf("Sent Message\n");
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
