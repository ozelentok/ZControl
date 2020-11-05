#include "TcpSocket.hpp"
#include "Commander.hpp"
#define FUSE_USE_VERSION 30
#include <fuse.h>
#include <fcntl.h>
#include <cstdio>
#include <cstring>

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

	int32_t dfd = commander.opendir("/tmp/ac");
	printf("opendir result: %d\n", dfd);
	auto dir_entries = commander.readdir(dfd, 20);
	for (const auto &dir_entry: dir_entries) {
		printf("dir entry: %d:%d:%s\n",
					 dir_entry.inode(),
					 dir_entry.type(), dir_entry.name().c_str());
	}
	int32_t closedir_result = commander.closedir(dfd);
	printf("closedir result: %d\n", closedir_result);
}

static int getattr_callback(const char *path, struct stat *stbuf) {
	memset(stbuf, 0, sizeof(struct stat));

	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		return 0;
	}

	return -ENOENT;
}

static int readdir_callback(const char *path, void *buf, fuse_fill_dir_t filler,
														off_t offset, struct fuse_file_info *fi) {
	(void) offset;
	(void) fi;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	return 0;
}

static int open_callback(const char *path, struct fuse_file_info *fi) {
	return 0;
}

static int read_callback(const char *path, char *buf, size_t size, off_t offset,
												 struct fuse_file_info *fi) {

	return -ENOENT;
}

static struct fuse_operations fuse_example_operations = {
	.getattr = getattr_callback,
	.open = open_callback,
	.read = read_callback,
	.readdir = readdir_callback,
};

int main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &fuse_example_operations, NULL);
}
