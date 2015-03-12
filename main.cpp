#include <iostream>
#include <fstream>
#include <regex>
#include <array>
#include <vector>

#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include "asd.h"

using namespace std;


class cfile {
public:
	string fname;
	string path;
	double mtime;
	vector<int> deps;
	cfile(string tfname, string tpath, double tmtime) 
			: fname(tfname), path(tpath), mtime(tmtime) { }
	string fpath() const {
		return path + "/" + fname;
	}
};

const string CC = "clang++ -Wall -std=c++11 -stdlib=libc++";
regex cppfile(".+\\.(cpp|hpp|c|h)");
regex cppsourcefile("(.+)\\.(cpp|c)");
regex includefile("\\s*#include\\s+\"(.+)\"");
string bin_files;

vector<cfile> files;
vector<string> args;
int currenttime = 0;


// helper - string to lower case
string tolower(string str) {
	for (auto &c : str)
		c = std::tolower(c);
	return str;
}


int filelist(string path) {
	DIR *dir;
	struct dirent *ent;
	struct stat st;

	if ((dir = opendir(path.c_str())) == NULL)
		return EXIT_FAILURE;  // could not open directory

	// print all the files and directories within directory
	while ((ent = readdir(dir)) != NULL) {
		if (string(".") == ent->d_name || string("..") == ent->d_name)
			continue;

		string filepath = path + "/" + ent->d_name;
		lstat(filepath.c_str(), &st);  // get file statistics

		// if we find a directory, get a file sub-list
		if (S_ISDIR(st.st_mode))
			filelist(filepath);
		// otherwise, add file info to the file list
		else {
			if (regex_match(ent->d_name, cppfile)) {
				files.push_back(cfile(
					ent->d_name,
					path,
					st.st_mtime
				));
			}
		}
	}

	closedir(dir);
	return 0;
}


int find_includes(cfile &cf) {
	// regex ex("\\s*#include\\s+([<\"])(.+)[>\"]\\s*");
	smatch match;
	string s;
	fstream file;
	file.open(cf.fpath());

	while (!file.eof()) {
		getline(file, s);
		if (regex_search(s, match, includefile)) {
			// cout << "\t" << match[1] << endl;
			
			// add file indexes to the list
			for (int i = 0; i < files.size(); i++) {
				if (files[i].fname == match[1])
					cf.deps.push_back(i);
			}
		}
	}

	file.close();
	return 0;
}


// latest modified file, uses file info
double latest_modtime(const cfile &cf) {
	double mtime = cf.mtime;
	for (auto i : cf.deps)
		if (files[i].mtime > mtime)
			mtime = files[i].mtime;
	return mtime;
}


// single file (anything, obj file)
double latest_modtime(const string &fpath) {
	struct stat st;
	int err = lstat(fpath.c_str(), &st);  // get file statistics
	if (err)
		return 0;  // oldest possible file :)
	return (double)st.st_mtime;
}


int is_source(const cfile &cf) {
	if (regex_match(cf.fname, cppsourcefile))
		return 1;
	return 0;
}


string cpp_base_fname(const cfile &cf) {
	smatch match;
	if (regex_search(cf.fname, match, cppsourcefile))
		return match[1];
	return "";
}


int compile(const cfile &cf) {
	string objname = "./bin/" + cpp_base_fname(cf) + ".o";
	bin_files += " " + objname;

	if (latest_modtime(objname) < latest_modtime(cf)) {
		string command = CC 
			+ " -I " + cf.path
			+ " -c -o " + objname 
			+ " " + cf.fpath();
		cout << command << endl;
		return system(command.c_str());
	}
	return 0;
}


int arguments(int argc, char** argv) {
	for (int i = 0; i < argc; i++) {
		string s = tolower(argv[i]);
		args.push_back(s);
	}
	return 0;
}


int has_arg(string arg) {
	for (auto a : args)
		if (a == arg)
			return 1;
	return 0;
}



int main(int argc, char** argv) {
	currenttime = time(NULL);
	arguments(argc, argv);

	if (has_arg("clean")) {
		cout << "cleaning bin files..." << endl;
		system("rm -rf bin");  // just delete bin and exit
		return 0;
	}

	system("mkdir -p bin");  // make bin directory
	filelist(".");  // get all base files

	if (files.size() == 0) {
		cerr << "no source files found." << endl;
		return 1;
	}

	cout << "source dependancies:" << endl;
	
	// calculate dependancies of all found files
	for (auto &f : files) {
		find_includes(f);
	}

	// display dependancies of this file
	for (auto &f : files) {
		cout << f.fpath() << "  [  ";
		for (auto i : f.deps)
			cout << files[i].fname << "  ";
		// cout << "]  " << (__int64_t)latest_modtime(f) << endl;
		cout << "]" << endl;
	}
	cout << endl;

	// compile all files
	for (auto &f : files) {
		int err = 0;
		if (is_source(f))
			err = compile(f);
		if (err)
			return 1;  // stop here if there was an error
	}

	// make the main executable
	string cmd = CC + bin_files + " -o ./bin/main.out";
	int err = system(cmd.c_str());
	if (err) {
		return err;
	} else if (has_arg("run")) {
		const char* s = "./bin/main.out";
		cout << "running: " << s << endl;
		err = system(s);
	}

	// return either main.out make error, or run error
	return err;
}
