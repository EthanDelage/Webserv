/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: edelage <edelage@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/03 13:34:00 by edelage           #+#    #+#             */
/*   Updated: 2023/09/03 13:34:00 by edelage          ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */
#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <stdint.h>
# include <vector>
# include <string>
# include <cstring>
# include <cstdlib>
# include <limits>
# include <cerrno>
# include "message/Message.hpp"
# include "method.hpp"

# define HTTP_HIGHEST_MAJOR_VERSION_SUPPORTED	1
# define HTTP_HIGHEST_MINOR_VERSION_SUPPORTED	1
# define BUFFER_SIZE							2048

class Request : public Message {

# ifdef UNIT_TESTING
	friend class RequestTest;

public:
	Request() {};
# endif

private:
	int				_clientSocket;
	uint8_t			_method;
	std::string		_requestURI;

	void	parseRequest();
	void	parseRequestLine();
	void	parseRequestHeader();
	void	parseRequestBody();
	void	parseMethod(std::string const & arg);
	void	parseHttpVersion(std::string const & arg);

	static uint8_t					getMethodByName(std::string const & method);
	static std::vector<std::string> split(std::string const & str);
	static std::string				getLine(int fd);

public:
	Request(int clientSocket);
	~Request();

	uint8_t			getMethod() const;
	std::string 	getRequestUri() const;

	void print() const;

};

#endif