#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <filesystem>
using namespace std;

class FileManager{
public:
	string read(string in_file_path);
	void output_by_line(vector<string> output, string out_file_path);
	vector<string> read_by_string(string in_file_path);
};

