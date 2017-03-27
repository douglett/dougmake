#ifndef __ARGS_DEF__
	#define __ARGS_DEF__

#include <string>

namespace args {
	const int VER=3;
	int parse(int argc, char** argv);
	int has_arg(std::string arg);
	int has_switch(std::string sw);
	int print_help();
	int print_version();
}

#endif