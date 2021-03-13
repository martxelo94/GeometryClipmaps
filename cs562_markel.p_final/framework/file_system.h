/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2019 Markel Pisano's Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of Markel Pisano Berrojalbiz is prohibited.
File Name: file_system.h
Purpose: utils on file managing
Language: c++
Platform: windows 10
Project: cs300_markel.p_0
Author: Markel Pisano Berrojalbiz
Creation date: 5/16/2019
----------------------------------------------------------------------------------------------------------*/
#pragma once

#include <string>
#include <vector>

//return null string if there is no extension (.extension)
std::string get_extension(const std::string &str);
//return null string if file not opened or corrupted
std::string read_to_string(const char* filename);
//overload to_string to set float precision
std::string to_string(float val, const unsigned precision = 3);
// return a list of file names in target folder
std::vector<std::string> get_filenames(const std::string& folder_name, const std::string extension = "");