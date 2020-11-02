#include "TcpSocket.hpp"
#include "Commander.hpp"
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>


void server() {
	TcpSocket server;
	server.bind("127.0.0.1", 4444);
	server.listen(1);
	printf("Listening on 127.0.0.1:4444\n");
	TcpSocket conn = server.accept();
	printf("Connection established\n");
	Commander commander(conn);
	uint32_t fd_id = commander.open("/tmp/a.txt", O_CREAT | O_RDWR);
	printf("fd_id: %d\n", fd_id);
	std::string buf = "Hello world\n";
	int32_t bytes_written = commander.write(fd_id, reinterpret_cast<const uint8_t*>(buf.c_str()), buf.length());
	printf("bytes written: %d\n", bytes_written);
	std::string buf2 = "Another write\n";
	bytes_written = commander.write(fd_id, reinterpret_cast<const uint8_t*>(buf2.c_str()), buf2.length());
	printf("bytes written: %d\n", bytes_written);
	int32_t close_result = commander.close(fd_id);
	printf("close result: %d\n", close_result);
}

int main(int argc, char const* argv[])
{
	try {
		while (true) {
			server();
		}
		return 0;
	} catch (std::exception& e) {
		printf("Exception: %s\n", e.what());
	}
}
