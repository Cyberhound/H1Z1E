#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

std::string local_path();
std::string read_file(std::string path);
void write_file(std::string path, std::string data);
bool directory_exists(std::string dir);
bool file_exists(std::string path);
bool copy_file(std::string from, std::string to);
bool delete_file(std::string path);
void create_directory(std::string dir);
std::vector<std::string> get_files(std::string dir, std::string extension = ".*");
std::vector<std::string> get_directories(std::string dir, bool folderNameOnly = false);