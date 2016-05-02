#ifndef __HELPERS_DEF__
	#define __HELPERS_DEF__

#include <string>

std::string tolower(std::string str);
std::string choppa(std::string str);

namespace platform {
	extern const std::string OS_STRING;
}

#endif