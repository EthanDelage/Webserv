/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: edelage <edelage@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/03 13:34:00 by edelage           #+#    #+#             */
/*   Updated: 2023/09/03 13:34:00 by edelage          ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */
#include "message/Request.hpp"
#include <iostream>

Request::Request() {

}

Request::~Request() {}

/**
 * @brief Parse line in the following format:
 * 			 Request-Line = Method SP Request-URI SP HTTP-Version CRLF
 */
void Request::parseRequestLine(std::string const & line) {
	std::vector<std::string>	argv;

	argv = split(line);
	if (argv.size() != 3)
		throw (std::runtime_error("Invalid number of arguments"));
	parseMethod(argv[0]);
	_requestURI = argv[1];
	parseHttpVersion(argv[2]);
}

void Request::parseMethod(std::string const & arg) {
	size_t		index = 0;
	std::string	method;

	while (arg[index] && arg[index] != ' ')
		++index;
	method = arg.substr(0, index);
	_method = getMethod(method);
}

/**
 * @brief Parse line in the following format:
 * 			 HTTP-Version = "HTTP" "/" 1*DIGIT "." 1*DIGIT
 */
void Request::parseHttpVersion(const std::string& arg) {
	char			*rest;
	char			*minor;
	unsigned long	conversion;

	if (arg.compare(0, strlen("HTPP/"), "HTTP/") != 0)
		throw (std::runtime_error(std::string("invalid format: `") + arg + '`'));
	conversion = strtoul(arg.c_str() + strlen("HTTP/"), &rest, 10);
	if (*rest != '.' || !std::isdigit(arg[strlen("HTTP/")]))
		throw (std::runtime_error("Invalid Major HTTP Version"));
	else if (errno == ERANGE || conversion > std::numeric_limits<unsigned int>::max())
		throw (std::runtime_error("Major HTTP Version overflow"));
	_httpVersion.major = static_cast<unsigned int>(conversion);
	minor = rest + 1;
	conversion = strtoul(minor, &rest, 10);
	if (*rest || !std::isdigit(minor[0]))
		throw (std::runtime_error("Invalid Minor HTTP Version"));
	else if (errno == ERANGE || conversion > std::numeric_limits<unsigned int>::max())
		throw (std::runtime_error("Minor HTTP Version overflow"));
	_httpVersion.minor = static_cast<unsigned int>(conversion);
	if (_httpVersion.major > HTTP_HIGHEST_MAJOR_VERSION_SUPPORTED
		|| (_httpVersion.major == HTTP_HIGHEST_MAJOR_VERSION_SUPPORTED
			&& _httpVersion.minor > HTTP_HIGHEST_MINOR_VERSION_SUPPORTED))
		throw (std::runtime_error("HTTP Version not supported"));
}

uint8_t Request::getMethod(const std::string& method) {
	std::string	methodList[] = {"GET", "POST", "DELETE", ""};
	uint8_t		methodMask[] = {GET_METHOD_MASK, POST_METHOD_MASK, DELETE_METHOD_MASK};

	for (size_t i = 0; methodList[i] != ""; ++i) {
		if (method == methodList[i])
			return (methodMask[i]);
	}
	throw(std::runtime_error("Unknown method"));
}

std::vector<std::string> Request::split(std::string const & str) {
	std::vector<std::string>	argv;
	std::string					arg;
	size_t						start;

	for (size_t i = 0; i < str.size(); i++) {
		if (str[i] == ' ')
			throw (std::runtime_error("split()"));
		start = i;
		while (str[i] && str[i] != ' ')
			i++;
		if (str[i] && str[i + 1] == '\0')
			throw (std::runtime_error("split()"));
		arg = str.substr(start, i - start);
		argv.push_back(arg);
	}
	return (argv);
}