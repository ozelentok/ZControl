#pragma once
#include <unistd.h>

class Pipe {
	private:
		int _pipe_fds[2];

	public:
		Pipe();
		Pipe(const Pipe &other) = delete;
		Pipe(Pipe &&other);
		Pipe& operator=(const Pipe &other) = delete;
		Pipe& operator=(Pipe &&other) = delete;
		~Pipe();
		int get_read_fd();
		int get_write_fd();
};
