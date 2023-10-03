/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: edelage <edelage@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/08 15:13:00 by edelage           #+#    #+#             */
/*   Updated: 2023/09/08 15:13:00 by edelage          ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */
#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "message/Request.hpp"
# include "config/VirtualServerConfig.hpp"
# include "config/LocationConfig.hpp"

class Response : public Message {
	typedef void (Response::*responseFunction_t)();

private:
	int					_clientSocket;
	std::string			_statusLine;
	Request				_request;
	LocationConfig*		_locationConfig;

	void router();
	void responseGet();
	void responsePost();
	void responseDelete();

	std::string 		getResourcePath();
	LocationConfig*		getResponseLocation(VirtualServerConfig const & virtualServerConfig);
	std::string			getContentType(std::string const & path) const;
	static void			send(int clientSocket, std::string statusLine, std::string header, std::string body);
	static std::string	getFileContent(std::ifstream& file);
	static std::string	statusCodeToLine(uint16_t statusCode);
	static std::string	httpVersionToString();
	static std::string	getReasonPhrase(uint16_t code);
	static std::string	statusCodeToString(unsigned int statusCode);
	static std::string	uitoa(unsigned int n);
	static bool			removeDirectory(std::string const & dirName);

public:
	Response(Request& request, VirtualServerConfig& virtualServerConfig);
	~Response();

	void send(int clientSocket);
	void print() const;

	static void sendContinue(int clientSocket);
	static void sendClientError(int clientSocket, std::string path);
};

#endif