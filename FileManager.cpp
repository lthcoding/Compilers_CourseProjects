#include "FileManager.h"

string FileManager::read(string in_file_path) {
	string line;
	ifstream in_file(in_file_path);
	string ret;
	if (in_file) {
		while (getline(in_file, line)) {
			ret.append(line);
			ret.push_back('\n');
		}
	}
	else {
		throw "Input File Not Found";
	}
	return ret;
}


void FileManager::output_by_line(vector<string> output, string out_file_path) {
	filesystem::path out_path = out_file_path;
	if (!filesystem::exists(out_path.parent_path())) {
		filesystem::create_directories(out_path.parent_path());
	}
	ofstream out_file(out_file_path);
	for (auto line : output) {
		out_file << line << endl;
	}
}


vector<string> FileManager::read_by_string(string in_file_path) {
	string line, tmp;
	bool e;
	e = filesystem::exists(in_file_path);
	ifstream in_file(in_file_path);
	vector<string> ret;
	stringstream ss;
	while (getline(in_file, line)) {
		stringstream ss(line);
		while (getline(ss, tmp, ' ')) {
			ret.push_back(tmp);
		}
	}
	return ret;
}
