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
# include "error/Error.hpp"

class Response : public Message {
	typedef void (Response::*responseFunction_t)();

private:
	std::string			_statusLine;
	Request				_request;
	LocationConfig*		_locationConfig;

	void router();
	void responseGet();
	void responsePost();
	void responseDelete();

	std::string 		getResourcePath();
	std::string			getContentType(std::string const & path) const;
	void				listingDirectory();
	void				setRequestBody();
	void 				addContentType(std::string const & path);
	bool				checkAcceptWildcard(std::string const & contentType, std::string const & acceptValue);
	static void			send(int clientSocket, std::string statusLine, std::string header, std::string body);
	static std::string	getFileContent(std::ifstream& file);
	static std::string	statusCodeToLine(uint16_t statusCode);
	static std::string	httpVersionToString();
	static std::string	getReasonPhrase(uint16_t code);
	static std::string	statusCodeToString(unsigned int statusCode);
	static std::string	uitoa(unsigned int n);
	static bool			removeDirectory(std::string const & dirName);

	bool				isDirectory(std::string const & path);
	bool				isFile(std::string const & path);

public:
	Response(Request& request);
	~Response();

	void send();
	void setDate();
	void print() const;

	void		responseRedirectionError(std::string const & pathErrorPage);
	static void sendContinue(int clientSocket);
	static void sendClientError(int statusCode, int clientSocket, clientException const & clientException);
	static void sendServerError(int statusCode, int clientSocket, std::string const & errorPagePath);
};

#endif