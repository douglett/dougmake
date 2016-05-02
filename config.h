#ifndef __CONFIG_DEF__
	#define __CONFIG_DEF__

#include <string>

namespace config {
	int load();
	int show_all();
	std::string pkg_config(std::string flags);
	std::string ccflags();
	std::string CC(std::string extension);
	std::string outfile();
}

#endif