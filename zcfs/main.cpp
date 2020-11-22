#include "TcpSocket.hpp"
#include "Commander.hpp"
#include "Server.hpp"
#include <memory>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#define FUSE_USE_VERSION 30
#include <fuse.h>

static std::unique_ptr<Server> server;
static const int64_t VirtualRootDirFd = -1;

static void* init_callback(struct fuse_conn_info *conn) {
	//TODO: add as parameters from main
	server = std::make_unique<Server>("0.0.0.0", 4444);
	server->start();
	return nullptr;
}

static void destroy_callback(void *private_data) {
	server->stop();
	server.reset(nullptr);
}

static std::pair<std::string, std::string> split_path(const char *path) {
	auto next_slash_index = ::strchr(path + 1, '/');
	if (next_slash_index == nullptr || next_slash_index == path + ::strlen(path) -1) {
		return std::make_pair<std::string, std::string>(path + 1, "/");
	}

	return std::make_pair<std::string, std::string>(
		std::string(path + 1, next_slash_index),
		std::string(next_slash_index));
}

static int getattr_callback(const char *path, struct stat *stbuf) {
	memset(stbuf, 0, sizeof(struct stat));
	try {
		if (strcmp(path, "/") == 0) {
			stbuf->st_mode = S_IFDIR | 0755;
			stbuf->st_nlink = 2;
			return 0;
		}

		auto [client, remote_path] = split_path(path);
		auto commander = server->get_commander(client);
		if (commander == nullptr) {
			return -ENOENT;
		}
		auto [result, worker_errno] = commander->getattr(remote_path, *stbuf);
		if (!result) {
			return -worker_errno;
		}
		return 0;
	} catch (...) {
		return -5;
	}
}

static int access_callback(const char *path, int mode) {
	if (strcmp(path, "/") == 0) {
		return 0;
	}

	auto [client, remote_path] = split_path(path);
	auto commander = server->get_commander(client);
	if (commander == nullptr) {
		return -ENOENT;
	}
	auto [result, worker_errno] = commander->access(remote_path, mode);
	if (!result) {
		return -worker_errno;
	}
	return 0;
}

static int opendir_callback(const char *path, struct fuse_file_info *fi) {
	if (strcmp(path, "/") == 0) {
		fi->fh = VirtualRootDirFd;
		return 0;
	}

	auto [client, remote_path] = split_path(path);
	auto commander = server->get_commander(client);
	if (commander == nullptr) {
		return -ENOENT;
	}
	auto [result, worker_errno] = commander->opendir(remote_path);
	if (result == -1) {
		return -worker_errno;
	}
	fi->fh = result;
	return 0;
}

static int releasedir_callback(const char *path, struct fuse_file_info *fi) {
	if (fi->fh == VirtualRootDirFd) {
		return 0;
	}

	auto [client, remote_path] = split_path(path);
	auto commander = server->get_commander(client);
	if (commander == nullptr) {
		return -ENOENT;
	}
	auto [result, worker_errno] = commander->closedir(fi->fh);
	if (result == -1) {
		return -worker_errno;
	}
	return 0;
}

static int readdir_callback(const char *path, void *buf, fuse_fill_dir_t filler,
														off_t offset, struct fuse_file_info *fi) {
	if (fi->fh == VirtualRootDirFd) {
		struct stat st = { 0 };
		st.st_mode = S_IFDIR | 0755;
		st.st_nlink = 2;
		filler(buf, ".", &st, 0);
		filler(buf, "..", &st, 0);
		auto clients = server->get_clients();
		for (const auto &c : clients) {
			filler(buf, c.c_str(), &st, 0);
		}
		return 0;
	}

	auto [client, remote_path] = split_path(path);
	auto commander = server->get_commander(client);
	if (commander == nullptr) {
		return -EIO;
	}
	auto [dirs, worker_errno] = commander->readdir(fi->fh, 40);
	if (worker_errno != 0) {
		return -worker_errno;
	}

	for (const DirEntry& e : dirs) {
		struct stat st = { 0 };
		st.st_mode = e.type() << 12;
		if (filler(buf, e.name().c_str(), &st, 0)) {
			printf("Filler failed\n");
			break;
		}
	}
	return 0;
}

static int open_callback(const char *path, struct fuse_file_info *fi) {
	if (strcmp(path, "/") == 0) {
		return -EISDIR;
	}

	auto [client, remote_path] = split_path(path);
	auto commander = server->get_commander(client);
	if (commander == nullptr) {
		return -ENOENT;
	}
	auto [result, worker_errno] = commander->open(remote_path, fi->flags);
	if (result == -1) {
		return -worker_errno;
	}
	fi->fh = result;
	return 0;
}

static int create_callback(const char *path, mode_t mode, struct fuse_file_info *fi) {
	if (strcmp(path, "/") == 0) {
		return -EISDIR;
	}

	auto [client, remote_path] = split_path(path);
	auto commander = server->get_commander(client);
	if (commander == nullptr) {
		return -ENOENT;
	}
	auto [result, worker_errno] = commander->open(remote_path, fi->flags, mode);
	if (result == -1) {
		return -worker_errno;
	}
	fi->fh = result;
	return 0;
}

static int release_callback(const char *path, struct fuse_file_info *fi) {
	auto [client, remote_path] = split_path(path);
	auto commander = server->get_commander(client);
	if (commander == nullptr) {
		return -ENOENT;
	}
	auto [result, worker_errno] = commander->close(fi->fh);
	if (result == -1) {
		return -worker_errno;
	}
	return 0;
}

static int read_callback(const char *path, char *buf, size_t size,
												off_t offset, struct fuse_file_info *fi) {
	auto [client, remote_path] = split_path(path);
	auto commander = server->get_commander(client);
	if (commander == nullptr) {
		return -EIO;
	}
	auto [result, worker_errno] = commander->pread(fi->fh, reinterpret_cast<uint8_t*>(buf), size, offset);
	if (result == -1) {
		return -worker_errno;
	}
	return result;
}

static int write_callback(const char *path, const char *buf, size_t size,
												off_t offset, struct fuse_file_info *fi) {
	auto [client, remote_path] = split_path(path);
	auto commander = server->get_commander(client);
	if (commander == nullptr) {
		return -EIO;
	}
	auto [result, worker_errno] = commander->pwrite(fi->fh, reinterpret_cast<const uint8_t*>(buf), size, offset);
	if (result == -1) {
		return -worker_errno;
	}
	return result;
}

static struct fuse_operations fuse_example_operations = {
	.getattr = getattr_callback,
	.open = open_callback,
	.read = read_callback,
	.write = write_callback,
	.release = release_callback,
	.opendir = opendir_callback,
	.readdir = readdir_callback,
	.releasedir = releasedir_callback,
	.init = init_callback,
	.destroy = destroy_callback,
	.access = access_callback,
	.create = create_callback,
};

int main(int argc, char *argv[]) {
	return fuse_main(argc, argv, &fuse_example_operations, NULL);
}
