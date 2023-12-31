/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServerConfigTest.hpp                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mururoah <mururoah@student.42lyon.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/08 21:29:00 by mururoah          #+#    #+#             */
/*   Updated: 2023/08/08 21:29:00 by mururoah         ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */
#ifndef VIRTUALSERVERCONFIGTEST_HPP
# define VIRTUALSERVERCONFIGTEST_HPP
# include <gtest/gtest.h>
# include "config/VirtualServerConfig.hpp"

class VirtualServerConfigTest : public ::testing::Test {

private:
	VirtualServerConfig virtualServerConfig;

protected:
	std::vector<std::string>		parseServerNameTest(char* line);
	socketAddress_t					parseListenTest(char* line);

	std::vector<std::string>	getServerName() const {return (virtualServerConfig._serverNames);}
	std::string					getAddress() const {return (virtualServerConfig._socketAddress.first);}
	uint16_t					getPort() const {return (virtualServerConfig._socketAddress.second);}
};

#endif