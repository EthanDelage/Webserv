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
#include <Config/Config.hpp>
#include "VirtualServerConfig.hpp"

#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cmath>

Config::Config() {
	_index.push_back(DEFAULT_INDEX);
	_isDefaultIndex = true;
	_root = std::string(PREFIX) + DEFAULT_ROOT;
	_errorPage[404] = "404.html";
	_maxBodySize = DEFAULT_MAX_BODY_SIZE;
	_autoindex = DEFAULT_AUTOINDEX;
}

Config::Config(Config const & other) {
	_index = other._index;
	_root = other._root;
	_errorPage = other._errorPage;
	_maxBodySize = other._maxBodySize;
	_autoindex = other._autoindex;
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
	size_t		separator;

	(void)configFile;
	separator = line.find(' ');
	directive = line.substr(0, separator);
	value = line.substr(separator + 1, line.size());
	try {
		Config::router(directive, value);
	}
	catch (std::exception const & e) {
		throw (std::runtime_error("Invalid format: `" + line + "`\n"
			+ e.what()));
	}
}

typedef void (Config::*parseFunctionType)(std::string&);

void Config::router(std::string& directive, std::string& value) {
    std::string 		directives[] = {"autoindex",
		"client_max_body_size", "error_page", "index", "root"};
    parseFunctionType	parseFunction[] = {&Config::parseAutoindex,
		&Config::parseMaxBodySize, &Config::parseErrorPage,
		&Config::parseIndex, &Config::parseRoot};

	for (size_t i = 0; i < (sizeof(directives) / sizeof(*directives)); i++) {
        if (directives[i] == directive) {
            (this->*parseFunction[i])(value);
            return;
        }
    }
	throw (std::runtime_error("Unknown command\n"));
}

void Config::parseAutoindex(std::string& value) {
	if (value == "on;")
		_autoindex = true;
	else if (value == "off;")
		_autoindex = false;
	else
		throw (std::runtime_error(SYNTAX_AUTOINDEX));
}

void Config::parseMaxBodySize(std::string& value) {
	ssize_t		convertValue;

	if (*(value.end() - 1) != ';' || value.size() == 1)
		throw (std::runtime_error(SYNTAX_MAX_BODY_SIZE));
	value.erase(value.end() - 1);
	convertValue = parseSize(value);
	if (convertValue == -1)
		throw (std::runtime_error(SYNTAX_SIZE));
	_maxBodySize = convertValue;
}

void Config::parseErrorPage(std::string &value) {
	std::string uri;

	if (*(value.end() - 1) != ';' || value.size() == 1)
		throw (std::runtime_error(SYNTAX_ERROR_PAGE));
    value.erase(value.end() - 1);
	uri = GetUriErrorPage(value);
	while (!value.empty()) {
		_errorPage[getNextErrorCode(value)] = uri;
		if (value.size() > 3 && value[3] != ' ')
			throw (std::runtime_error(SYNTAX_ERROR_PAGE));
		if (value.size() > 4)
			value.erase(0, 4);
		else
			value.erase(0, 3);
	}
}

void Config::parseIndex(std::string &value) {
	std::string	file;

	if (*(value.end() - 1) != ';' || value.size() == 1)
		throw (std::runtime_error(SYNTAX_INDEX));
    value.erase(value.end() - 1);
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

	if (*(value.end() - 1) != ';' || value.size() == 1)
		throw (std::runtime_error(SYNTAX_ROOT));
    value.erase(value.end() - 1);
	_root = parsePath(value);
}

void Config::parseServer(std::ifstream &configFile) {
	(void) configFile;
}

std::string Config::parsePath(std::string &value) {
	std::string	path;

	path = removeQuote(value);
	if (path[0] != '/')
		path = PREFIX + path;
	return (path);
}

/**
 * @brief converts a string in 'size' format to ssize_t
 * @return returns converted value or -1 on error
 */
ssize_t Config::parseSize(std::string &value) {
	char*	rest;
	double	conversion = std::strtod(value.c_str(), &rest);
	ssize_t	result = static_cast<ssize_t>(conversion);

	if (!std::isdigit(value[0])
		|| conversion != round(conversion)
		|| value.find('.') != std::string::npos)
		return (-1);
	if (*rest == '\0')
		return (result);
	else if (*rest == 'k' && *(++rest) == '\0')
		return (result * 1 << 10);
	else if (*rest == 'm' && *(++rest) == '\0')
		return (result * 1 << 20);
	else
		return (-1);
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

/**
 * @brief extract the next file and removes the quotes
 */
std::string Config::getNextFile(std::string &value) {
	std::string	file;
	size_t		i;

	for (i = 0; i < value.size() && value[i] != ' '; i++) {
		if (value[i] == '"')
		{
			i++;
			while (i < value.size() && value[i] != '"')
				i++;
		}
		else if (value[i] == '\'')
		{
			i++;
			while (i < value.size() && value[i] != '\'')
				i++;
		}
	}
	file = value.substr(0, i);
	value.erase(0, i + 1);
	return (removeQuote(file));
}

/**
 * @brief get the next code format: code = 3DIGIT
 */
uint16_t Config::getNextErrorCode(std::string &value) {
	uint16_t	code = 0;

	if (value[0] == ' ')
		throw (std::runtime_error("Codes must be separate by single space"));
	for (int i = 0; i < 3; ++i) {
		if (!std::isdigit(value[i]))
			throw (std::runtime_error("Syntax: code = 3DIGIT"));
		code = code * 10 + (value[i] - '0');
	}
	if (code < 300 || code > 599)
		throw (std::runtime_error('`' + value.substr(0, 3)
			+ "`: code must be between 300 and 599"));
	return (code);
}

/**
 * @brief get the uri and remove it from value
 * @return uri
 */
std::string Config::GetUriErrorPage(std::string &value) {
	size_t		indexLastSpace = 0;
	char		quote = 0;
	std::string	uri;

	for (size_t i = 0; i != value.size(); ++i) {
		if (value[i] == ' ' && quote == 0)
			indexLastSpace = i;
		else if ((value[i] == '\'' || value[i] == '"')
			&& (i == 0 || value[i - 1] != '\\')
			&& quote == 0)
			quote = value[i];
		else if ((value[i] == '\'' || value[i] == '"')
			&& (i == 0 || value[i - 1] != '\\')
			&& quote == value[i])
			quote = 0;
	}
	if (indexLastSpace == 0)
		throw (std::runtime_error("Invalid number of arguments"));
	else if (indexLastSpace == value.size() - 1)
		throw (std::runtime_error("Space after uri"));
	uri = value.substr(indexLastSpace + 1);
	value.erase(indexLastSpace);
	return (removeQuote(uri));
}

#include <iostream>

void Config::print() {
	std::cout << "Autoindex: " << _autoindex << std::endl;
	std::cout << "Max body size: " << _maxBodySize << std::endl;
	std::cout << "Error pages: " << _errorPage.begin()->second << std::endl;
	std::cout << "Index:";
	for (std::vector<std::string>::iterator i = _index.begin(); i !=  _index.end(); i++)
		std::cout << " | " << *i;
	std::cout << std::endl;
	std::cout << "Root: " << _root << std::endl;
}
