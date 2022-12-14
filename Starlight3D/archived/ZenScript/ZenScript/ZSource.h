#pragma once
#include <string>
#include <fstream>
#include <vector>
#include <iostream>

class ZSource
{
public:

	ZSource(std::string path);
	std::vector<std::string> GetCode();
	void Clean();

private:

	std::string mPath;
	std::vector<std::string> code;

};

