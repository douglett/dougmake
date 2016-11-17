#include <iostream>
#include <fstream>
#include <string>
#include <map>

#include "helpers.h"
#include "config.h"

using namespace std;


namespace config {

	static map<string, string> config = {
#if defined __clang__
		// clang defaults
		{ "cc", "clang++ -std=c++11 -Wall" },  // sensible default C++ compiler
		{ "cc1", "clang -std=c99 -Wall" },  // sensible default C compiler
#else
		// defaults to __GCC__ . fuck other compilers
		{ "cc", "g++ -std=c++11 -Wall -Wno-sign-compare" },  // sensible default C++ compiler
		{ "cc1", "gcc -std=c99 -Wall" },  // sensible default C compiler
#endif
		{ "out", "main.out" }  // sensible main filename 
	};


	// get config information from "dmake.conf" file
	int load() {
		// check for conf file
		fstream file;	
		file.open("dmake.conf");
		if (!file.is_open())
			return 0;

		// add config into a key:value array
		string s;
		while (getline(file, s)) {
			int i = s.find(":");
			if (i == string::npos)
				continue;
			string key = tolower(choppa( s.substr(0, i) ));
			string val = choppa( s.substr(i+1) );
			config[key] = val;
		}

		file.close();
		return 1;
	}


	// helper - useful to debug the conf file
	int show_all() {
		cout << "showing config options:" << endl;
		for (auto c : config)
			cout << c.first << " :: " << c.second << endl;
		return 0;
	}


	string pkg_config(string flags) {
		// find out if pkg-config exists
		const string& os = platform::OS_STRING_EXT;
		string pkg;
		if      (config.count("pkg-config-"+os))  pkg = "pkg-config-"+os;
		else if (config.count("pkg-config"))      pkg = "pkg-config";
		else    return "";
		// if it exists, get the correct flags
		if      (flags == "cflags")  return string(" `pkg-config --cflags ") + config[pkg] + "`";
		else if (flags == "libs")    return string(" `pkg-config --libs ") + config[pkg] + "`";
		else    return "";
	}


	string ccflags() {
		const string& os = platform::OS_STRING_EXT;
		if (config.count("ccflags-"+os))  return " " + config["ccflags-"+os];
		if (config.count("ccflags"))      return " " + config["ccflags"];
		return "";
	}


	string ldflags() {
		const string& os = platform::OS_STRING_EXT;
		if (config.count("ldflags-"+os))  return " " + config["ldflags-"+os];
		if (config.count("ldflags"))      return " " + config["ldflags"];
		return "";
	}


	string CC(string extension) {
		const string& os = platform::OS_STRING_EXT;
		string ext = ( extension == "c" ? "cc1" : "cc" );  // select c or cpp compiler
		if (config.count(ext+"-"+os))  return config[ext+"-"+os];  // platform specific compiler
		return config[ext];
	}


	string outfile() {
		return config["out"];
	}


} // end config
