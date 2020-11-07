#include "TcpSocket.hpp"
#include "Commander.hpp"
#define FUSE_USE_VERSION 30
#include <fuse.h>
#include <fcntl.h>
#include <cstdio>
#include <cstring>
#include <memory>
#include <unistd.h>


std::thread conn_thread;
std::unique_ptr<Commander> commander;

void server() {
	TcpSocket server;

	server.bind("0.0.0.0", 4444);
	server.listen(1);
	printf("Listening on 0.0.0.0:4444\n");
	commander = std::make_unique<Commander>(server.accept());
	printf("Connection established\n");
	pid_t pid = getpid();
	printf("SERVER PID: %d\n", pid);
};

static void* init_callback(struct fuse_conn_info *conn) {
	conn_thread = std::thread(server);
	return nullptr;
}

static int getattr_callback(const char *path, struct stat *stbuf) {
	memset(stbuf, 0, sizeof(struct stat));

	const bool result = commander->getattr(path, *stbuf);
	if (!result) {
		return -commander->last_errno();
	}
	return 0;
	// if (strcmp(path, "/") == 0) {
	// 	stbuf->st_mode = S_IFDIR | 0755;
	// 	stbuf->st_nlink = 2;
	// 	return 0;
	// }
}

static int readdir_callback(const char *path, void *buf, fuse_fill_dir_t filler,
														off_t offset, struct fuse_file_info *fi) {
	(void) offset;
	(void) fi;

	auto dfd = commander->opendir(path);
	if (dfd == -1) {
		return -commander->last_errno();
	}
	auto dirs = commander->readdir(dfd, 40);
	if (commander->last_errno() != 0) {
		return -commander->last_errno();
	}

	for (const DirEntry& e : dirs) {
		struct stat st = { 0 };
		st.st_mode = e.type() << 12;
		if (filler(buf, e.name().c_str(), &st, 0)) {
			printf("Filler failed\n");
			break;
		}
	}
	commander->closedir(dfd);
	return 0;
}

static int open_callback(const char *path, struct fuse_file_info *fi) {
	auto val = commander->open(path, fi->flags);
	if (val == -1) {
		return -commander->last_errno();
	}
	fi->fh = val;
	return 0;
}

static int create_callback(const char *path, mode_t mode, struct fuse_file_info *fi) {
	auto val = commander->open(path, fi->flags, mode);
	if (val == -1) {
		return -commander->last_errno();
	}
	fi->fh = val;
	return 0;
}

static int read_callback(const char *path, char *buf, size_t size,
												off_t offset, struct fuse_file_info *fi) {
	int32_t fd = -1;
	if (fi == nullptr) {
		fd = commander->open(path, O_RDONLY);
	} else {
		fd = fi->fh;
	}

	auto val = commander->pread(fd, reinterpret_cast<uint8_t*>(buf), size, offset);
	if (val == -1) {
		return -commander->last_errno();
	}

	if (fi == nullptr) {
		commander->close(fd);
	}

	return val;
}

static int write_callback(const char *path, const char *buf, size_t size,
												off_t offset, struct fuse_file_info *fi) {
	int32_t fd = -1;
	if (fi == nullptr) {
		fd = commander->open(path, O_WRONLY);
	} else {
		fd = fi->fh;
	}

	auto val = commander->pwrite(fd, reinterpret_cast<const uint8_t*>(buf), size, offset);
	if (val == -1) {
		return -commander->last_errno();
	}

	if (fi == nullptr) {
		commander->close(fd);
	}

	return val;
}

static struct fuse_operations fuse_example_operations = {
	.getattr = getattr_callback,
	.open = open_callback,
	.read = read_callback,
	.write = write_callback,
	.readdir = readdir_callback,
	.init = init_callback,
	.create = create_callback,
};

int main(int argc, char *argv[])
{
	pid_t pid = getpid();
	printf("MAIN PID: %d\n", pid);
	return fuse_main(argc, argv, &fuse_example_operations, NULL);
}
