#include "TcpSocket.hpp"
#include <cstdio>
#include <unistd.h>

void send_http_request() {
	uint8_t data[] = "GET / HTTP/1.1\r\nHost:127.0.0.1\r\n\r\n";
	TcpSocket sock;
	sock.connect("127.0.0.1", 80);
	sock.send(data, sizeof(data) / sizeof(data[0]));

	std::vector<uint8_t> buf(2048);
	sock.recv(buf.data(), buf.capacity());
	buf[buf.size()-1]= '\0';
	std::printf("%s\n", buf.data());
}

void simple_http_server() {
	TcpSocket sock;
	sock.bind("127.0.0.1", 4444);
	sock.listen(1);
	printf("Listening on 127.0.0.1:4444\n");
	TcpSocket conn = sock.accept();

	for (int i = 0; i < 5; i++) {
		std::vector<uint8_t> buf(2048);
		conn.send(reinterpret_cast<const uint8_t*>("1234"), sizeof(4));
		printf("Sleeping\n");
		sleep(5);
		size_t bytes_read = conn.recv(buf.data(), buf.capacity());
		buf[bytes_read] = 0;
		printf("%s\n", buf.data());
	}
}

int main(int argc, char const* argv[])
{
	try {

		simple_http_server();
		return 0;

	} catch (std::exception& e) {
		printf("Exception: %s\n", e.what());
	}
}
