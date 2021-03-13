/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2019 Markel Pisano's Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of Markel Pisano Berrojalbiz is prohibited.
File Name: file_systems.cpp
Purpose: implement file management utils
Language: c++
Platform: windows 10
Project: cs300_markel.p_0
Author: Markel Pisano Berrojalbiz
Creation date: 5/16/2019
----------------------------------------------------------------------------------------------------------*/

#include "file_system.h"
#include <iostream>	// std::cerr
#include <fstream>	// std::ifstream
#include <sstream>	// std::ostringstream
#include <Windows.h>

//return null string if there is no extension (.extension)
std::string get_extension(const std::string &str)
{
	try{
		return str.substr(str.find_last_of('.'));
	}
	catch (std::out_of_range){
		return "";
	}
}
//return null string if file not opened or corrupted
std::string read_to_string(const char* filename)
{
	std::ifstream file;
	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		file.open(filename);
		std::string res((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();
		return res;
	}
	catch (std::ifstream::failure e) {
		std::cerr << "Exception opening/reading/closing file \"" << filename << "\"\n";
		return "";
	}
}

//overload to_string to set float precision
std::string to_string(float val, const unsigned precision)
{
	std::ostringstream out;
	out.precision(precision);
	out << std::fixed << val;
	return out.str();
}

// return a list of file names in target folder
std::vector<std::string> get_filenames(const std::string& folder_name, const std::string extension)
{
	std::vector<std::string> filenames;
	WIN32_FIND_DATAA data;	// prevent using unicode
	HANDLE hFind;
	if ((hFind = FindFirstFileA(std::string(folder_name + "/*." + extension).c_str(), &data)) != INVALID_HANDLE_VALUE) {
		do {
			filenames.push_back(data.cFileName);
		} while (FindNextFileA(hFind, &data) != 0);
		FindClose(hFind);
	}
	return filenames;
}