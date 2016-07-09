#include <iostream>
#include <vector>

#include "args.h"
#include "helpers.h"

using namespace std;


namespace args {
	
	static vector<string> args;


	// get command line arguments into array 'args'
	int parse(int argc, char** argv) {
		for (int i = 0; i < argc; i++) {
			string s = tolower(argv[i]);  // store all arguments && switches in lower case
			args.push_back(s);
		}
		return 0;
	}


	// helper - returns true if argument 'arg' is in array 'args'
	int has_arg(string arg) {
		for (const auto &a : args)
			if (a == arg)
				return 1;
		return 0;
	}


	// helper - returns true if switch 'sw' exists in args array
	int has_switch(string sw) {
		int start;
		string str;
		for (const auto &a : args) {
			if (a[0] != '-')
				continue;
			start = ( a[1] == '-' ? 2 : 1 );
			str = a.substr(start);
			if (str == sw)
				return 1;
		}
		return 0;
	}


	// print a help message
	int print_help() {
		if (args::has_switch("help") || args::has_switch("?") || args::has_switch("h")) {
			cout << "options: -?, clean, rebuild, run" << endl;
			cout << "dmake.conf options:" << endl
				<< "\t" << "cc: [C compiler]" << endl
				<< "\t" << "pkg-config: [package names]" << endl
				<< "\t" << "ccflags: [manual compile flags]" << endl
				<< "\t" << "ldflags: [manual linker flags]" << endl;
			return 1;
		}
		return 0;
	}

}
