/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigTest.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: edelage <edelage@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/23 23:42:00 by edelage           #+#    #+#             */
/*   Updated: 2023/07/23 23:42:00 by edelage          ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */
#include "ConfigTest.hpp"

TEST_F(ConfigTest, testParseAutoindexOn) {
	ASSERT_EQ(parseAutoindexTest("autoindex on;"), true);
}

TEST_F(ConfigTest, testParseAutoindexOff) {
	ASSERT_EQ(parseAutoindexTest("autoindex off;"), false);
}

TEST_F(ConfigTest, testParseAutoindexInvalidInput) {
	EXPECT_EQ(parseAutoindexTest(" autoindex off;"), -1);
	EXPECT_EQ(parseAutoindexTest("autoindex  off;"), -1);
	EXPECT_EQ(parseAutoindexTest("autoindex off; "), -1);
	EXPECT_EQ(parseAutoindexTest("autoindex off"), -1);
	EXPECT_EQ(parseAutoindexTest("autoindex on"), -1);
	EXPECT_EQ(parseAutoindexTest("autoindexon"), -1);
	EXPECT_EQ(parseAutoindexTest("autoindex ;"), -1);
	EXPECT_EQ(parseAutoindexTest("autoindex no;"), -1);
	EXPECT_EQ(parseAutoindexTest("autoindex ono;"), -1);
	EXPECT_EQ(parseAutoindexTest("auto index ono;"), -1);
}

TEST_F(ConfigTest, testParseMaxBodySizeValid) {
	EXPECT_EQ(parseMaxBodySize("client_max_body_size 1024;"), 1024);
	EXPECT_EQ(parseMaxBodySize("client_max_body_size 1k;"), 1024);
	EXPECT_EQ(parseMaxBodySize("client_max_body_size 1m;"), 1 << 20);
}

TEST_F(ConfigTest, testParseMaxBodySizeInvalid) {
	EXPECT_THROW(parseMaxBodySize("client_max_body_size ;"), std::runtime_error);
	EXPECT_THROW(parseMaxBodySize("client_max_body_size m;"), std::runtime_error);
	EXPECT_THROW(parseMaxBodySize("client_max_body_size k;"), std::runtime_error);
	EXPECT_THROW(parseMaxBodySize("client_max_body_size 1024M;"), std::runtime_error);
	EXPECT_THROW(parseMaxBodySize("client_max_body_size 1024K;"), std::runtime_error);
	EXPECT_THROW(parseMaxBodySize("client_max_body_size 1024ko;"), std::runtime_error);
	EXPECT_THROW(parseMaxBodySize("client_max_body_size 1024mo;"), std::runtime_error);
}

int ConfigTest::parseAutoindexTest(char* line) {
	try {
		std::string lineStr(line);

		config.parseAutoindex(lineStr);
		return (config._autoindex);
	} catch (std::exception const & e) {
		return (-1);
	}
}

size_t ConfigTest::parseMaxBodySize(char *line) {
	std::string	lineStr(line);

	config.parseMaxBodySize(lineStr);
	return (config._maxBodySize);
}
