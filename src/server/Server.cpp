/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: edelage <edelage@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/14 11:18:00 by edelage           #+#    #+#             */
/*   Updated: 2023/08/14 11:18:00 by edelage          ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */
#include "server/Server.hpp"

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <algorithm>

Server::Server() {
	_socketArray = NULL;
	_nbSocket = 0;
}

Server::~Server() {
	if (_socketArray != NULL) {
		for (size_t i = 0; i < _nbSocket; ++i) {
			if (_socketArray[i].fd == -1)
				close(_socketArray[i].fd);
		}
		delete[] _socketArray;
	}
}

void Server::init(Config const & config) {
	getAddressArray(config.getServerConfig());
	_config = config;
	_nbSocket = _addressArray.size();
	_socketArray = new pollfd[_nbSocket];
	for (size_t i = 0; i < _nbSocket; ++i) {
		_socketArray[i].fd = -1;
	}
	for (size_t i = 0; i < _nbSocket; ++i) {
		try {
			_socketArray[i].fd = initSocket(_addressArray[i]);
			_socketArray[i].events = POLLIN;
			_socketArray[i].revents = POLL_DEFAULT;
		} catch (std::runtime_error const & e) {
			throw (std::runtime_error(std::string("initSocket() failed: ") + e.what()));
		}
	}
}

void Server::listener() {
	int	clientSocketFd;

	while (true) {
		if (poll(_socketArray, _nbSocket, POLL_TIMEOUT) == -1)
			throw (std::runtime_error("poll() failed"));
		for (size_t i = 0; i < _nbSocket; ++i) {
			if (_socketArray[i].revents != POLL_DEFAULT) {
				clientSocketFd = acceptClient(_socketArray[i].fd);
				(void) clientSocketFd;
			}
		}
	}
}

void	Server::getAddressArray(std::vector<VirtualServerConfig*> serverConfig) {
	socketAddress_t										socketAddress;
	std::vector<VirtualServerConfig*>::const_iterator 	it;

	for (it = serverConfig.begin(); it != serverConfig.end(); ++it) {
		socketAddress = (*it)->getSocketAddress();
		if (socketAddress.first == "*")
			socketAddress.first = ANY_ADDRESS;
		if (std::find(_addressArray.begin(), _addressArray.end(), socketAddress) == _addressArray.end())
			_addressArray.push_back(socketAddress);
	}
}

int Server::initSocket(socketAddress_t const & socketAddress) {
	int 				socketFd;
	struct sockaddr_in	address;
	int					opt = 1;

	socketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socketFd == -1)
		throw (std::runtime_error("socket() failed"));
	if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		throw (std::runtime_error("setsockopt() failed"));
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(socketAddress.first.c_str());
	address.sin_port = htons(socketAddress.second);
	if (bind(socketFd, (struct sockaddr*) &address, sizeof(address)) == -1) {
		close(socketFd);
		throw (std::runtime_error("bind() failed"));
	}
	if (listen(socketFd, QUEUE_LENGTH) == -1) {
		close(socketFd);
		throw(std::runtime_error("listen() failed"));
	}
	return (socketFd);
}

#include "iostream"
int Server::acceptClient(int socketFd) {
	int 				clientSocketFd;
	struct sockaddr_in	address;
	socklen_t			addressLength;

	clientSocketFd = accept(socketFd, (struct sockaddr*) &address, &addressLength);
	if (clientSocketFd == -1)
		throw (std::runtime_error("accept() failed"));
	std::cout << ntohs(address.sin_port) << std::endl;
	return (clientSocketFd);
}