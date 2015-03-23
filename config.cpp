#include <iostream>
#include <fstream>
#include <string>
#include <map>

#include "helpers.h"
#include "config.h"

using namespace std;


namespace config {

	static map<string, string> config;


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
		for (auto c : config)
			cout << c.first << " :: " << c.second << endl;
		return 0;
	}


} // end config
