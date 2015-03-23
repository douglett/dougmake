#include "helpers.h"

using namespace std;


// helper - string to lower case
string tolower(string str) {
	for (auto &c : str)
		c = std::tolower(c);
	return str;
}


// helper - chomp off leading/trailing whitespace
string choppa(string str) {
	string ws = " \t\n\r";
	int first = str.find_first_not_of(ws);
	if (first == string::npos)
		return "";  // str is all whitespace
	str.erase(0, first);
	int last = str.find_last_not_of(ws);
	if (last != string::npos)
		str.erase(last+1);
	return str;
}
