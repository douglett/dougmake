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
		if (!config.count("pkg-config"))
			return "";  // no pkg-config set
		else if (flags == "cflags")
			return string(" `pkg-config --cflags ") + config["pkg-config"] + "`";
		else if (flags == "libs")
			return string(" `pkg-config --libs ") + config["pkg-config"] + "`";
		return "";  // unknown option - just return nothing
	}


	string ccflags() {
		if (!config.count("ccflags"))
			return "";
		return " " + config["ccflags"];
	}


	string ldflags() {
		if (!config.count("ldflags"))
			return "";
		return " " + config["ldflags"];
	}


	string CC(string extension) {
		string cc = config["cc"];
		if (extension == "c")
			cc = config["cc1"];
		return cc;
	}


	string outfile() {
		return config["out"];
	}


} // end config
