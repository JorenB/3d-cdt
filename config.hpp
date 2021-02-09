// Copyright 2020 Joren Brunekreef and Andrzej Görlich
#pragma once

#include <fstream>
#include <string>
#include <cassert>
#include <unordered_map>

class ConfigReader {
public:
	void read(std::string fname) {
		std::ifstream infile(fname);
		assert(infile.is_open());
		std::string key, value;

		while (infile >> key >> value) {
			dict[key] = value;
		}

		assert(dict.find("k0") != dict.end());
		assert(dict.find("genus") != dict.end());
		assert(dict.find("targetVolume") != dict.end());
		assert(dict.find("target2Volume") != dict.end());
		assert(dict.find("seed") != dict.end());
		assert(dict.find("fileID") != dict.end());
		assert(dict.find("measurements") != dict.end());
//DN
assert(dict.find("thermal") != dict.end());
assert(dict.find("ksteps") != dict.end());
assert(dict.find("infile") != dict.end());
assert(dict.find("k3") != dict.end());
//assert(dict.find("outfile") != dict.end());
//ND

	}

	int getInt(std::string key) {
		return std::stoi(dict[key]);
	}

	double getDouble(std::string key) {
		return std::stod(dict[key]);
	}

	std::string getString(std::string key) {
		return dict[key];
	}

private:
	std::unordered_map<std::string, std::string> dict;
};
