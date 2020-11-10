#include "Server.hpp"

Server::Server(const std::string &host, uint16_t port) {
	_server.bind(host, port);
	//TODO: Server listen how many
	_server.listen(1);
}

Server::~Server() {
	try {
		_server.close();
	} catch (...) {}
}

void Server::start() {
	_should_stop = false;
	_acceptor_thread = std::thread(&Server::_accept_connections, this);
}

void Server::stop() {
	_should_stop = true;
	_server.close();
	_acceptor_thread.join();
}

void Server::_accept_connections() {
	try {
		while (!_should_stop) {
			auto [connection, client_ip, client_port] = _server.accept();
			printf("Client: %s\n", TcpSocket::format_connection(client_ip, client_port).c_str());
			std::lock_guard<std::mutex> lock(_commanders_mx);
			_commanders.emplace(TcpSocket::format_connection(client_ip, client_port),
													std::make_unique<Commander>(std::move(connection)));
		}
	} catch (...) {
		if (!_should_stop) {
			printf("Error while accepting connection\n");
		}
	}
}

std::shared_ptr<Commander> Server::get_commander(const std::string &client) {
	std::lock_guard<std::mutex> lock(_commanders_mx);
	auto it = _commanders.find(client);
	if (it == _commanders.end()) {
		return nullptr;
	}
	auto &commander_ptr = it->second;
	if (!commander_ptr->is_connected()) {
		_commanders.erase(it);
		return nullptr;
	}
	return it->second;
}

std::vector<std::string> Server::get_clients() {
	std::vector<std::string> clients;
	std::lock_guard<std::mutex> lock(_commanders_mx);
	for (const auto &it : _commanders) {
		if (it.second->is_connected()) {
			clients.push_back(it.first);
		}
	}
	return clients;
}
