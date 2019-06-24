// Copyright (c) Jérémy Ansel 2014, 2019
// Licensed under the MIT license. See LICENSE.txt

#include "config.h"

#include <string>
#include <fstream>
#include <algorithm>
#include <cctype>

Config g_config;

Config::Config()
{
	this->AspectRatioPreserved = false;

	std::ifstream file("Smush.cfg");

	if (file.is_open())
	{
		std::string line;

		while (std::getline(file, line))
		{
			if (!line.length())
			{
				continue;
			}

			if (line[0] == '#' || line[0] == ';' || (line[0] == '/' && line[1] == '/'))
			{
				continue;
			}

			int pos = line.find("=");

			std::string name = line.substr(0, pos);
			name.erase(std::remove_if(name.begin(), name.end(), std::isspace), name.end());

			std::string value = line.substr(pos + 1);
			value.erase(std::remove_if(value.begin(), value.end(), std::isspace), value.end());

			if (!name.length() || !value.length())
			{
				continue;
			}

			if (name == "PreserveAspectRatio")
			{
				this->AspectRatioPreserved = std::stoi(value) != 0;
			}
		}
	}
}
