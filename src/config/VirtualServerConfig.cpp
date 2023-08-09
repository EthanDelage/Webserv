/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServerConfig.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ethan <ethan@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/09 02:30:00 by ethan             #+#    #+#             */
/*   Updated: 2023/07/09 02:30:00 by ethan            ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */
#include "VirtualServerConfig.hpp"
#include "LocationConfig.hpp"
#include <cstdlib>
#include <limits>

#ifdef UNIT_TESTING
VirtualServerConfig::VirtualServerConfig() : Config() {
	_isDefault = true;
	_address = DEFAULT_ADDRESS;
	_port = DEFAULT_PORT;
}
#endif

VirtualServerConfig::VirtualServerConfig(Config const & config) : Config(config) {
	_isDefault = true;
	_address = DEFAULT_ADDRESS;
	_port = DEFAULT_PORT;
}

VirtualServerConfig::VirtualServerConfig(VirtualServerConfig const & other) : Config() {
	_index = other._index;
	_isDefaultIndex = other._isDefaultIndex;
	_root = other._root;
	_errorPage = other._errorPage;
	_maxBodySize = other._maxBodySize;
	_autoindex = other._autoindex;
	_isDefault = other._isDefault;
	_serverNames = other._serverNames;
	_address = other._address;
	_port = other._port;
	_locationConfig = other._locationConfig;
}

void VirtualServerConfig::parse(std::ifstream& configFile) {
	std::string	line;

		while (!configFile.eof())
	{
		std::getline(configFile, line);
		if (!line.empty())
			parseLine(line);
	}
}

void VirtualServerConfig::parseLine(std::string& line) {
	std::string	directive;
	std::string	value;

	lineLexer(line, directive, value);
	try {
		router(directive, value);
	}
	catch (std::exception const & e) {
		throw (std::runtime_error("Invalid format: `" + line + "`\n"
			+ e.what()));
	}
}

void VirtualServerConfig::router(std::string& directive, std::string& value) {
	std::string	directives[] = {
		"autoindex",
		"client_max_body_size",
		"error_page",
		"index",
		"listen",
		"root",
		"server_name"
	};
	parseFunctionType	parseFunction[] = {
		&VirtualServerConfig::parseAutoindex,
		&VirtualServerConfig::parseMaxBodySize,
		&VirtualServerConfig::parseErrorPage,
		&VirtualServerConfig::parseIndex,
		&VirtualServerConfig::parseListen,
		&VirtualServerConfig::parseRoot,
		&VirtualServerConfig::parseServerName,
	};

	for (size_t i = 0; i < (sizeof(directives) / sizeof(*directives)); i++) {
		if (directives[i] == directive) {
			(this->*parseFunction[i])(value);
			return;
		}
	}
	throw (std::runtime_error("Unknown command\n"));
}

void VirtualServerConfig::parseListen(std::string& value) {
	double						conversion;
	char*						endptr;
	size_t						separator;
	std::vector<std::string>	argv;

	argv = split(value, SYNTAX_LISTEN);
	if (argv.size() != 1)
		throw (std::runtime_error(SYNTAX_LISTEN));
	separator = value.find(':');
	if (separator != std::string::npos)
	{
		_address = value.substr(0, separator);
		value.erase(0,separator + 1);
		if (value.empty())
			throw (std::runtime_error(SYNTAX_LISTEN));
	}
	if (!isValidIP(value)) {
		conversion = getPort(value);
		_port = static_cast<uint8_t>(conversion);
	}
}

void VirtualServerConfig::parseServerName(std::string& value) {
	std::vector<std::string>			argv;
	std::vector<std::string>::iterator	it;

	argv = Config::split(value, SYNTAX_SERVER_NAME);
	for (it = argv.begin(); it != argv.end(); it++)
		_serverNames.push_back(*it);
}

#include <iostream>
bool	VirtualServerConfig::isValidIP(std::string const & str) {
	size_t	index = 0;

	try {
		getIpByte(str, index);
	} catch (std::runtime_error const & e) {
		return (false);
	}
	for (int i = 0; i < 3; ++i) {
		if (str[index] != '.')
			return (false);
		index++;
		try {
			getIpByte(str, index);
		} catch (std::runtime_error const & e) {
			return (false);
		}
	}
	std::cout << "TRUE" << std::endl;
	return (true);
}

uint8_t VirtualServerConfig::getIpByte(std::string const & address, size_t& index) {
	uint8_t	byte = 0;

	if (!std::isdigit(address[index]))
		throw (std::runtime_error(SYNTAX_ADDRESS));
	for (int i = 0; i < 3 && std::isdigit(address[index]); ++i) {
		std::cout << address.c_str() + index << " -> " << (int) byte << std::endl;
		if ((byte * 10 + (address[index] - '0')) / 10 != byte) {
			std::cout << "throw" << std::endl;
			throw (std::runtime_error(SYNTAX_ADDRESS));
		}
		byte = byte * 10 + (address[index] - '0');
		++index;
		std::cout << "test" << std::endl;
	}
	if (std::isdigit(address[index]))
		throw (std::runtime_error(SYNTAX_ADDRESS));
	std::cout << address << " -> " << (int) byte << std::endl;
	std::cout << "endl" << std::endl;
	return (byte);
}

uint16_t VirtualServerConfig::getPort(std::string const & str) {
	double	conversion;
	char*	rest;

	conversion = std::strtod(str.c_str(), &rest);
	if (*rest != '\0'
		|| conversion != round(conversion)
		|| conversion > std::numeric_limits<uint16_t>::max()
		|| conversion < 0)
		throw (std::runtime_error(SYNTAX_PORT));
	return (static_cast<uint16_t>(conversion));
}

#include <iostream>

void VirtualServerConfig::print() {
	std::cout << "VIRTUAL SERVER" << std::endl;
	Config::print();
	std::cout << "Port: " << _port << std::endl;
	std::cout << "Address: " << _address << std::endl;
	std::cout << "Server Names: ";
	for (std::vector<std::string>::iterator i = _serverNames.begin(); i !=  _serverNames.end(); i++)
		std::cout << *i << " | ";
	std::cout << std::endl;
}
