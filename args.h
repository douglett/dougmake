#ifndef __ARGS_DEF__
	#define __ARGS_DEF__

#include <string>

namespace args {
	int parse(int argc, char** argv);
	int has_arg(std::string arg);
	int print_help();
}

#endif