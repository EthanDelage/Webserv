/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ethan <ethan@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/07 16:43:00 by ethan             #+#    #+#             */
/*   Updated: 2023/07/07 16:43:00 by ethan            ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */
#include "config/Config.hpp"
#include "config/VirtualServerConfig.hpp"

#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <sstream>
#include "utils.hpp"

Config::Config() {
	_index.push_back(DEFAULT_INDEX);
	_isDefaultIndex = true;
	_root = std::string(PREFIX) + DEFAULT_ROOT;
	_errorPage[300] = _root + '/' + "300.html";
	_errorPage[400] = _root + '/' + "400.html";
	_errorPage[500] = _root + '/' + "500.html";
	_maxBodySize = DEFAULT_MAX_BODY_SIZE;
	_autoindex = DEFAULT_AUTOINDEX;
}

Config::Config(Config const & other) {
	_autoindex = other._autoindex;
	_maxBodySize = other._maxBodySize;
	_errorPage = other._errorPage;
	_index = other._index;
	_isDefaultIndex = other._isDefaultIndex;
	_root = other._root;
	_types = other._types;
	_cgi = other._cgi;
	_cgiFolder = other._cgiFolder;
}

Config::~Config() {
	for (std::vector<VirtualServerConfig*>::iterator it = _serverConfig.begin(); it != _serverConfig.end(); ++it) {
		delete *it;
	}
}

std::vector<VirtualServerConfig*> Config::getServerConfig() const {return (_serverConfig);}

std::string Config::getRoot() const {return (_root);}

std::vector<std::string>	Config::getIndex() const {return (_index);}

std::map<std::string, std::string> Config::getTypes() const {return (_types);}

bool Config::getAutoindex() const {return (_autoindex);}

std::map<uint16_t, std::string> Config::getErrorPage() const {return (_errorPage);}

size_t Config::getMaxBodySize() const {return (_maxBodySize);}

std::string Config::getCgiFolder() const {return (_cgiFolder);}

std::vector<std::string> Config::getCgi() const {return (_cgi);}

VirtualServerConfig* Config::getDefaultServer(socketAddress_t const & socketAddress) const {
	socketAddress_t	serverSocketAddress;

	for (std::vector<VirtualServerConfig*>::const_iterator it = _serverConfig.begin(); it != _serverConfig.end(); ++it) {
		serverSocketAddress = (*it)->getSocketAddress();
		if (serverSocketAddress == socketAddress
			|| (serverSocketAddress.first == DEFAULT_ADDRESS
				&& serverSocketAddress.second == socketAddress.second))
			return (*it);
	}
	return (_serverConfig[0]);
}

void Config::parse(char* configFilename) {
	std::ifstream	configFile(configFilename);
	std::string		line;

	if (!configFile.is_open())
		throw (std::runtime_error(std::string("Couldn't open file: ") + configFilename));
	while (!configFile.eof()) {
		std::getline(configFile, line);
		if (!line.empty())
			parseLine(line, configFile);
	}
	configFile.close();
}

void Config::parseLine(std::string& line, std::ifstream& configFile) {
	std::string	directive;
	std::string	value;

	if (line == "server {")
	{
		parseServer(configFile);
		return ;
	}
	lineLexer(line, directive, value);
	try {
		Config::router(directive, value);
	}
	catch (std::exception const & e) {
		throw (std::runtime_error("Invalid format: `" + line + "`\n"
			+ e.what()));
	}
}

void Config::lineLexer(std::string& line, std::string& directive, std::string& value) {
	size_t		separator;

	separator = line.find(' ');
	if (separator == std::string::npos) {
		std::cout << std::endl << line << std::endl;
		throw (std::runtime_error("Invalid format: `" + line + "`\n"
			+ "Unknown command"));
	}
	directive = line.substr(0, separator);
	value = line.substr(separator + 1, line.size());
	if (*(value.end() - 1) != ';' || value.size() == 1)
		throw (std::runtime_error("Invalid format: `" + line + "`\n"
			+ "Missing `;`"));
	value.erase(value.size() - 1);
}

void Config::router(std::string& directive, std::string& value) {
    std::string directives[] = {
		"autoindex",
		"cgi",
		"client_max_body_size",
		"error_page",
		"index",
		"root",
		"type"
	};
    parseFunctionType	parseFunction[] = {
		&Config::parseAutoindex,
		&Config::parseCgi,
		&Config::parseMaxBodySize,
		&Config::parseErrorPage,
		&Config::parseIndex,
		&Config::parseRoot,
		&Config::parseType
	};

	for (size_t i = 0; i < (sizeof(directives) / sizeof(*directives)); i++) {
		if (directives[i] == directive) {
			(this->*parseFunction[i])(value);
			return;
		}
	}
	throw (std::runtime_error("Unknown command"));
}

void Config::parseAutoindex(std::string& value) {
	if (value == "on")
		_autoindex = true;
	else if (value == "off")
		_autoindex = false;
	else
		throw (std::runtime_error(SYNTAX_AUTOINDEX));
}

void Config::parseMaxBodySize(std::string& value) {
	size_t		convertValue;

	convertValue = parseSize(value);
	_maxBodySize = convertValue;
}

void Config::parseErrorPage(std::string &value) {
	std::vector<std::string>			argv;
	std::string							uri;
	std::vector<std::string>::iterator	it;
	uint16_t							errorCode;

	argv = split(value, SYNTAX_ERROR_PAGE);
	if (argv.size() <= 1)
		throw (std::runtime_error(SYNTAX_ERROR_PAGE));
	uri = argv.back();
	argv.pop_back();
	for (it = argv.begin(); it != argv.end(); it++) {
		errorCode = getErrorCode(*it);
		if (errorCode % 100 == 0)
			_errorPage[errorCode] = _root + '/' + uri;
		else
			warnErrorCode(errorCode);
	}
}

void Config::parseIndex(std::string &value) {
	std::string	file;

	if (_isDefaultIndex == true) {
		_isDefaultIndex = false;
		_index.clear();
	}
	while (!value.empty()) {
		file = getNextFile(value);
		if (file.empty())
			throw (std::runtime_error(SYNTAX_INDEX));
		_index.push_back(file);
	}
}

void Config::parseRoot(std::string &value) {
	std::vector<std::string>	args;

	args = split(value, SYNTAX_ROOT);
	if (args.size() != 1)
		throw (std::runtime_error(SYNTAX_ROOT));
	_root = removeQuote(args[0]);
	if (_root[0] != '/')
		_root = PREFIX + _root;
}

void Config::parseServer(std::ifstream &configFile) {
	VirtualServerConfig*	newServerConfig = new VirtualServerConfig(*this);

	try {
		newServerConfig->parse(configFile);
		_serverConfig.push_back(newServerConfig);
	} catch (std::runtime_error const & e) {
		delete newServerConfig;
		throw (e);
	}
}

void Config::parseType(std::string& value) {
	std::vector<std::string>	args;
	std::string					contentType;

	args = split(value, SYNTAX_TYPE);
	if (args.size() < 2)
		throw (std::runtime_error(SYNTAX_TYPE));
	contentType = removeQuote(args[0]);
	if (!isValidContentType(contentType))
		throw (std::runtime_error(SYNTAX_TYPE));
	for (size_t i = 1; i != args.size(); ++i) {
		_types[toLower(removeQuote(args[i]))] = contentType;
	}
}

void Config::parseCgi(std::string& value) {
	std::vector<std::string>			args;
	std::vector<std::string>::iterator 	it;

	args = split(value, SYNTAX_CGI);
	if (args.size() < 2)
		throw (std::runtime_error(SYNTAX_CGI));
	_cgiFolder = args[0];
	_cgi.clear();
	for (it = args.begin() + 1; it != args.end(); ++it) {
		if (!isValidCgiFilename(*it))
			throw (std::runtime_error(SYNTAX_CGI));
		_cgi.push_back(*it);
	}
}

bool Config::isValidContentType(const std::string& contentType) {
	std::string type;
	std::string subtype;
	size_t		indexSeparator;

	indexSeparator = contentType.find('/');
	if (indexSeparator == std::string::npos)
		return (false);
	type = contentType.substr(0, indexSeparator);
	subtype = contentType.substr(indexSeparator + 1);
	if (!isValidToken(type) || !isValidToken(subtype))
		return (false);
	return (true);
}

bool Config::isValidCgiFilename(std::string& filename) {
	size_t 		indexExtension;
	std::string	extension;

	indexExtension = filename.rfind('.');
	if (indexExtension == std::string::npos || indexExtension == 0)
		return (false);
	extension = filename.substr(indexExtension);
	if (extension != ".py" && extension != ".php")
		return (false);
	return (true);
}

bool Config::isValidToken(std::string const & token) {
	for (size_t i = 0; i < token.size(); ++i) {
		if (strchr(SEPARATORS, token[i]) != NULL || !isprint(token[i]))
			return (false);
	}
	return (true);
}

/**
 * @brief converts a 'size' formatted string to size_t
 * @return the converted value
 */
size_t Config::parseSize(std::string &value) {
	char*	rest;
	double	conversion = std::strtod(value.c_str(), &rest);
	size_t	result = static_cast<size_t>(conversion);

	if (!std::isdigit(value[0])
		|| conversion != round(conversion)
		|| value.find('.') != std::string::npos)
		throw (std::runtime_error(SYNTAX_SIZE));
	if (*rest == '\0')
		return (result);
	else if (*rest == 'k' && *(++rest) == '\0')
		return (result * 1 << 10);
	else if (*rest == 'm' && *(++rest) == '\0')
		return (result * 1 << 20);
	else
		throw (std::runtime_error(SYNTAX_SIZE));
}

VirtualServerConfig* Config::findServerConfig(socketAddress_t const & socketAddress, std::string host) const {
	std::vector<VirtualServerConfig*>	serverConfig;
	VirtualServerConfig*				result;
	uint8_t								bestScore;
	uint8_t								currentScore;

	if (host.find(':') != std::string::npos)
		host.erase(host.find(':'));
	if (isValidIP(host))
		host.erase();
	serverConfig = findServerConfigBySocketAddress(socketAddress);
	if (!serverConfig.empty())
		serverConfig = findServerConfigByHost(serverConfig, toLower(host));
	else
		return (getDefaultServer(socketAddress));
	bestScore = 0;
	for (std::vector<VirtualServerConfig*>::const_iterator it = serverConfig.begin(); it != serverConfig.end(); ++it) {
		currentScore = serverConfigGetScore(*it, host);
		if (currentScore > bestScore) {
			bestScore = currentScore;
			result = *it;
		}
	}
	if (bestScore == 0)
		return (getDefaultServer(socketAddress));
	return (result);
}

std::vector<VirtualServerConfig*> Config::findServerConfigBySocketAddress(socketAddress_t const & socketAddress) const {
	std::vector<VirtualServerConfig*>	serverConfig;
	socketAddress_t						currentSocketAddress;

	for (std::vector<VirtualServerConfig*>::const_iterator it = _serverConfig.begin(); it != _serverConfig.end(); ++it) {
		currentSocketAddress = (*it)->getSocketAddress();
		if (currentSocketAddress.first == DEFAULT_ADDRESS && currentSocketAddress.second == DEFAULT_PORT)
			serverConfig.push_back(*it);
		else if (currentSocketAddress.first == DEFAULT_ADDRESS && socketAddress.second == currentSocketAddress.second)
			serverConfig.push_back(*it);
		else if (socketAddress.first == currentSocketAddress.first && currentSocketAddress.second == DEFAULT_PORT)
			serverConfig.push_back(*it);
		else if (socketAddress == currentSocketAddress) {
			serverConfig.push_back(*it);
		}
	}
	return (serverConfig);
}

std::vector<VirtualServerConfig*>
	Config::findServerConfigByHost(std::vector<VirtualServerConfig*> serverConfig, std::string const & host) {
	std::vector<VirtualServerConfig*>	result;
	std::vector<std::string>			currentServerNames;

	for (std::vector<VirtualServerConfig*>::const_iterator it = serverConfig.begin(); it != serverConfig.end(); ++it) {
		currentServerNames = (*it)->getServerNames();
		if (std::find(currentServerNames.begin(), currentServerNames.end(), host) != currentServerNames.end())
			result.push_back(*it);
	}
	return (result);
}

uint8_t Config::serverConfigGetScore(VirtualServerConfig *serverConfig, std::string const & host) {
	uint8_t	score;

	score = PORT_SCORE;
	if (serverConfig->getIp() != DEFAULT_ADDRESS)
		score += IP_SCORE;
	if (!host.empty())
		score += HOST_SCORE;
	return (score);
}

/**
 * @brief takes a string and removes quotes (", ').
 *  Also handles \" and \'
 */
std::string Config::removeQuote(std::string &str) {
	std::string	result;
	char		quote;

	for (size_t i = 0; i < str.size(); ++i) {
		if ((str[i] == '"' || str[i] == '\'')
			&& (i == 0 || str[i - 1] != '\\')) {
			quote = str[i];
			++i;
			while (i < str.size() && (str[i] != quote
					|| (str[i] == quote && str[i - 1] == '\\'))) {
				if (str[i] == quote)
					*(result.end() - 1) = quote;
				else
					result += str[i];
				++i;
			}
			if (i == str.size())
				throw (std::runtime_error(std::string("Missing: `")
					+ quote + '`'));
		} else if ((str[i] == '"' || str[i] == '\'') && str[i -1] == '\\')
			*(result.end() - 1) = str[i];
		else
			result += str[i];
	}
	return (result);
}

void Config::warnErrorCode(uint16_t code) {
	std::stringstream warnMessage;

	warnMessage << "Webserv: [warn] error code `"
		<< static_cast<int>(code)
		<< "` is not handled, ignored" << std::endl;
	printColor(std::cerr, warnMessage.str(), RED);
}

/**
 * @brief extract the next file and removes the quotes
 */
std::string Config::getNextFile(std::string &value) {
	std::string	file;
	size_t		i;

	for (i = 0; i < value.size() && value[i] != ' '; i++) {
		skipQuotes(value, i);
	}
	if (value[i] == ' ' && value[i + 1] == '\0')
		throw (std::runtime_error(SYNTAX_INDEX));
	file = value.substr(0, i);
	value.erase(0, i + 1);
	return (removeQuote(file));
}

/**
 * @brief get the next code format: code = 3DIGIT
 */
uint16_t Config::getErrorCode(std::string &value) {
	uint16_t	code = 0;

	if (value.size() != 3)
		throw (std::runtime_error(SYNTAX_ERROR_CODE));
	for (int i = 0; i < 3; ++i) {
		if (!std::isdigit(value[i]))
			throw (std::runtime_error(SYNTAX_ERROR_CODE));
		code = code * 10 + (value[i] - '0');
	}
	if (code < 300 || code > 599)
		throw (std::runtime_error("code must be between 300 and 599"));
	return (code);
}

void Config::skipQuotes(std::string& str, size_t& index) {
	if (str[index] == '"' && (index == 0 || str[index - 1] != '\\'))
	{
		++index;
		while (index < str.size()
			&& str[index] != '"' && str[index - 1] != '\\')
			++index;
	}
	else if (str[index] == '\'' && (index == 0 || str[index - 1] != '\\'))
	{
		++index;
		while (index < str.size()
			&& str[index] != '\'' && str[index - 1] != '\\')
			++index;
	}
}

std::vector<std::string> Config::split(std::string& str, std::string const syntax) {
	std::vector<std::string>	argv;
	size_t						start;
	std::string					arg;

	if (str[0] == ' ' || str[str.size() - 1] == ' ')
		throw (std::runtime_error(syntax));
	for (size_t i = 0; i < str.size(); ++i) {
		start = i;
		while (str[i] && str[i] != ' ') {
			skipQuotes(str, i);
			++i;
		}
		arg = str.substr(start, i - start);
		removeQuote(arg);
		argv.push_back(arg);
		if (str[i] && str[i + 1] == ' ')
			throw (std::runtime_error(syntax));
	}
	return (argv);
}

bool Config::isValidIP(std::string const & str) const {
	size_t	index = 0;

	if (!isValidIpByte(str, index))
		return (false);
	for (int i = 0; i < 3; ++i) {
		if (str[index] != '.')
			return (false);
		index++;
		if (!isValidIpByte(str, index))
			return (false);
	}
	if (str[index])
		return (false);
	return (true);
}

bool Config::isValidIpByte(std::string const & address, size_t& index) const {
	uint8_t	byte = 0;

	if (!std::isdigit(address[index]))
		return (false);
	for (int i = 0; i < 3 && std::isdigit(address[index]); ++i) {
		if (((uint8_t) (byte * 10 + (address[index] - '0'))) / 10 != byte)
			return (false);
		byte = byte * 10 + (address[index] - '0');
		++index;
	}
	if (std::isdigit(address[index]))
		return (false);
	return (true);
}
