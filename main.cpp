#include "TcpSocket.hpp"
#include <cstdio>

int main(int argc, char const* argv[])
{
	try {
		uint8_t data[] = "GET / HTTP/1.1\r\nHost:127.0.0.1\r\n\r\n";
		TcpSocket sock;
		sock.connect("127.0.0.1", 80);
		sock.send(data, sizeof(data) / sizeof(data[0]));

		std::vector<uint8_t> buf(2048);
		sock.recv(buf.data(), buf.capacity());
		buf[buf.size()-1]= '\0';
		std::printf("%s\n", buf.data());
		return 0;
	} catch (std::exception& e) {
		printf("%s\n", e.what());
	}
}
