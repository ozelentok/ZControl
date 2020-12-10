#include "TcpSocket.hpp"
#include "Commander.hpp"
#include "Server.hpp"
#include <memory>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/vfs.h>
#include <syslog.h>
#define FUSE_USE_VERSION 31
#include <fuse.h>

static std::unique_ptr<Server> server;
static const int64_t VirtualRootDirFd = -1;

static void* zcfs_init(struct fuse_conn_info *conn, struct fuse_config *cfg) {
	//TODO: add as parameters from main
	server = std::make_unique<Server>("0.0.0.0", 4444);
	server->start();
	return nullptr;
}

static void zcfs_destroy(void *private_data) {
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

#define BEGIN_ZCFS_ERROR_HANDLER  \
	try {
#define END_ZCFS_ERROR_HANDLER(error_value)                           \
	} catch (const std::exception &e) {                                 \
		syslog(LOG_ERR, "Error on %s(%s): %s", __func__, path, e.what());  \
		return -error_value;                                              \
	} catch (...) {                                                     \
		syslog(LOG_ERR, "Unknwon Error on %s(%s)", __func__, path);       \
		return -error_value;                                              \
	}
#define END_ZCFS_ERROR_RENAME_HANDLER(error_value)                           \
	} catch (const std::exception &e) {                                 \
		syslog(LOG_ERR, "Error on %s(%s, %s): %s", __func__, old_path, new_path, e.what());  \
		return -error_value;                                              \
	} catch (...) {                                                     \
		syslog(LOG_ERR, "Unknwon Error on %s(%s, %s)", __func__, old_path, new_path);       \
		return -error_value;                                              \
	}

static int zcfs_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
	memset(stbuf, 0, sizeof(*stbuf));
	BEGIN_ZCFS_ERROR_HANDLER
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
	END_ZCFS_ERROR_HANDLER(EIO)
}

static int zcfs_mkdir(const char *path, mode_t mode) {
	BEGIN_ZCFS_ERROR_HANDLER
	auto [client, remote_path] = split_path(path);
	auto commander = server->get_commander(client);
	if (commander == nullptr) {
		return -ENOENT;
	}
	auto [result, worker_errno] = commander->mkdir(remote_path, mode);
	if (!result) {
		return -worker_errno;
	}
	return 0;
	END_ZCFS_ERROR_HANDLER(EIO)
}

static int zcfs_unlink(const char *path) {
	BEGIN_ZCFS_ERROR_HANDLER
	auto [client, remote_path] = split_path(path);
	auto commander = server->get_commander(client);
	if (commander == nullptr) {
		return -ENOENT;
	}
	auto [result, worker_errno] = commander->unlink(remote_path);
	if (!result) {
		return -worker_errno;
	}
	return 0;
	END_ZCFS_ERROR_HANDLER(EIO)
}

static int zcfs_rmdir(const char *path) {
	BEGIN_ZCFS_ERROR_HANDLER
	auto [client, remote_path] = split_path(path);
	auto commander = server->get_commander(client);
	if (commander == nullptr) {
		return -ENOENT;
	}
	auto [result, worker_errno] = commander->rmdir(remote_path);
	if (!result) {
		return -worker_errno;
	}
	return 0;
	END_ZCFS_ERROR_HANDLER(EIO)
}

static int zcfs_rename(const char *old_path, const char *new_path, unsigned int flags) {
	BEGIN_ZCFS_ERROR_HANDLER
	auto [old_client, old_remote_path] = split_path(old_path);
	auto [new_client, new_remote_path] = split_path(old_path);
	if (old_client != new_client) {
		return -EXDEV;
	}

	auto commander = server->get_commander(old_client);
	if (commander == nullptr) {
		return -ENOENT;
	}
	auto [result, worker_errno] = commander->rename(old_remote_path, new_remote_path, flags);
	if (!result) {
		return -worker_errno;
	}
	return 0;
	END_ZCFS_ERROR_RENAME_HANDLER(EIO)
}

static int zcfs_chmod(const char *path, mode_t mode, struct fuse_file_info *fi) {
	BEGIN_ZCFS_ERROR_HANDLER
	auto [client, remote_path] = split_path(path);
	auto commander = server->get_commander(client);
	if (commander == nullptr) {
		return -ENOENT;
	}
	auto [result, worker_errno] = commander->chmod(remote_path, mode);
	if (!result) {
		return -worker_errno;
	}
	return 0;
	END_ZCFS_ERROR_HANDLER(EIO)
}

static int zcfs_chown(const char *path, uid_t owner, gid_t group, struct fuse_file_info *fi) {
	BEGIN_ZCFS_ERROR_HANDLER
	auto [client, remote_path] = split_path(path);
	auto commander = server->get_commander(client);
	if (commander == nullptr) {
		return -ENOENT;
	}
	auto [result, worker_errno] = commander->chown(remote_path, owner, group);
	if (!result) {
		return -worker_errno;
	}
	return 0;
	END_ZCFS_ERROR_HANDLER(EIO)
}

static int zcfs_truncate(const char *path, off_t size, struct fuse_file_info *fi) {
	BEGIN_ZCFS_ERROR_HANDLER
	auto [client, remote_path] = split_path(path);
	auto commander = server->get_commander(client);
	if (commander == nullptr) {
		return -ENOENT;
	}
	auto [result, worker_errno] = commander->truncate(remote_path, size);
	if (!result) {
		return -worker_errno;
	}
	return 0;
	END_ZCFS_ERROR_HANDLER(EIO)
}

static int zcfs_open(const char *path, struct fuse_file_info *fi) {
	BEGIN_ZCFS_ERROR_HANDLER
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
	END_ZCFS_ERROR_HANDLER(EIO)
}

static int zcfs_read(const char *path, char *buf, size_t size,
												off_t offset, struct fuse_file_info *fi) {
	BEGIN_ZCFS_ERROR_HANDLER
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
	END_ZCFS_ERROR_HANDLER(EIO)
}

static int zcfs_write(const char *path, const char *buf, size_t size,
												off_t offset, struct fuse_file_info *fi) {
	BEGIN_ZCFS_ERROR_HANDLER
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
	END_ZCFS_ERROR_HANDLER(EIO)
}

static int zcfs_statfs(const char *path, struct statvfs *stbuf) {
	BEGIN_ZCFS_ERROR_HANDLER
	memset(stbuf, 0, sizeof(*stbuf));
	if (strcmp(path, "/") == 0) {
		stbuf->f_flag = ST_NOATIME | ST_NODIRATIME | ST_NOSUID;
		return 0;
	}

	auto [client, remote_path] = split_path(path);
	auto commander = server->get_commander(client);
	if (commander == nullptr) {
		return -ENOENT;
	}
	auto [result, worker_errno] = commander->statvfs(remote_path, stbuf);
	if (!result) {
		return -worker_errno;
	}
	stbuf->f_flag |= ST_NOATIME | ST_NODIRATIME | ST_NOSUID;
	return 0;
	END_ZCFS_ERROR_HANDLER(EIO)
}

static int zcfs_release(const char *path, struct fuse_file_info *fi) {
	BEGIN_ZCFS_ERROR_HANDLER
	auto [client, remote_path] = split_path(path);
	auto commander = server->get_commander(client);
	if (commander == nullptr) {
		return -ENOENT;
	}
	auto [result, worker_errno] = commander->close(fi->fh);
	if (!result) {
		return -worker_errno;
	}
	return 0;
	END_ZCFS_ERROR_HANDLER(EIO)
}

static int zcfs_opendir(const char *path, struct fuse_file_info *fi) {
	BEGIN_ZCFS_ERROR_HANDLER
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
	END_ZCFS_ERROR_HANDLER(EIO)
}

static int zcfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
														off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
	BEGIN_ZCFS_ERROR_HANDLER
	if (fi->fh == VirtualRootDirFd) {
		struct stat st = { 0 };
		st.st_mode = S_IFDIR | 0755;
		st.st_nlink = 2;
		filler(buf, ".", &st, 0, static_cast<fuse_fill_dir_flags>(0));
		filler(buf, "..", &st, 0, static_cast<fuse_fill_dir_flags>(0));
		auto clients = server->get_clients();
		for (const auto &c : clients) {
			filler(buf, c.c_str(), &st, 0, static_cast<fuse_fill_dir_flags>(0));
		}
		return 0;
	}

	auto [client, remote_path] = split_path(path);
	auto commander = server->get_commander(client);
	if (commander == nullptr) {
		return -EIO;
	}
	// TODO: Call readdir in a loop for directories with more than 1024 entries
	auto [dirs, worker_errno] = commander->readdir(fi->fh, 1024);
	if (worker_errno != 0) {
		return -worker_errno;
	}

	for (const DirEntry& e : dirs) {
		struct stat st = { 0 };
		st.st_mode = e.type() << 12;
		if (filler(buf, e.name().c_str(), &st, 0, static_cast<fuse_fill_dir_flags>(0))) {
			syslog(LOG_WARNING, "readdir(%s) buffer is full", path);
			break;
		}
	}
	return 0;
	END_ZCFS_ERROR_HANDLER(EIO)
}

static int zcfs_releasedir(const char *path, struct fuse_file_info *fi) {
	BEGIN_ZCFS_ERROR_HANDLER
	if (fi->fh == VirtualRootDirFd) {
		return 0;
	}

	auto [client, remote_path] = split_path(path);
	auto commander = server->get_commander(client);
	if (commander == nullptr) {
		return -ENOENT;
	}
	auto [result, worker_errno] = commander->closedir(fi->fh);
	if (!result) {
		return -worker_errno;
	}
	return 0;
	END_ZCFS_ERROR_HANDLER(EIO)
}

static int zcfs_access(const char *path, int mode) {
	BEGIN_ZCFS_ERROR_HANDLER
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
	END_ZCFS_ERROR_HANDLER(EIO)
}

static int zcfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
	BEGIN_ZCFS_ERROR_HANDLER
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
	END_ZCFS_ERROR_HANDLER(EIO)
}

static int zcfs_utimens(const char *path, const struct timespec times[2], struct fuse_file_info *fi) {
	BEGIN_ZCFS_ERROR_HANDLER
	auto [client, remote_path] = split_path(path);
	auto commander = server->get_commander(client);
	if (commander == nullptr) {
		return -ENOENT;
	}
	auto [result, worker_errno] = commander->utimens(remote_path, times);
	if (!result) {
		return -worker_errno;
	}
	return 0;
	END_ZCFS_ERROR_HANDLER(EIO)
}

static struct fuse_operations zcfs_operations = {
	.getattr = zcfs_getattr,
	.mkdir = zcfs_mkdir,
	.unlink = zcfs_unlink,
	.rmdir = zcfs_rmdir,
	.rename = zcfs_rename,
	.chmod = zcfs_chmod,
	.chown = zcfs_chown,
	.truncate = zcfs_truncate,
	.open = zcfs_open,
	.read = zcfs_read,
	.write = zcfs_write,
	.statfs = zcfs_statfs,
	.release = zcfs_release,
	.opendir = zcfs_opendir,
	.readdir = zcfs_readdir,
	.releasedir = zcfs_releasedir,
	.init = zcfs_init,
	.destroy = zcfs_destroy,
	.access = zcfs_access,
	.create = zcfs_create,
	.utimens = zcfs_utimens,
};

int main(int argc, char *argv[]) {
	return fuse_main(argc, argv, &zcfs_operations, NULL);
}
