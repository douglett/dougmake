#include <iostream>
#include <fstream>
#include <regex>
#include <vector>

#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#include "helpers.h"
#include "config.h"
#include "args.h"

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
		if (path == ".")
			return fname;
		return path + "/" + fname;
	}
};

const regex cppfile(".+\\.(cpp|hpp|c|h)");
const regex cppsourcefile("(.+)\\.(cpp|c)");
const regex includefile("\\s*#include\\s+\"(.+)\"");

const string txt_cyan = "\e[0;36m";
const string txt_reset = "\e[0m";

// string CC = "clang++ -std=c++11 -stdlib=libc++  -Wall";  // sensible default
vector<cfile> files;
string bin_files;
int currenttime = 0;



// --- helpers ---



// helper - true for "*.cpp"
int is_source(const cfile &cf) {
	if (regex_match(cf.fname, cppsourcefile))
		return 1;
	return 0;
}

// helper - returns (fname).cpp from filename
string cpp_base_fname(const cfile &cf) {
	smatch match;
	if (regex_search(cf.fname, match, cppsourcefile))
		return match[1];
	return "";
}

// helper - latest modified file, uses file info
double latest_modtime(const cfile &cf) {
	double mtime = cf.mtime;
	for (auto i : cf.deps)
		if (files[i].mtime > mtime)
			mtime = files[i].mtime;
	return mtime;
}

// helper - latest modified file. takes a single file (anything, obj file)
double latest_modtime(const string &fpath) {
	struct stat st;
	int err = stat(fpath.c_str(), &st);  // get file statistics
	if (err)
		return 0;  // file doesn't exist, return oldest possible file time
	return (double)st.st_mtime;
}



// --- parse actions ---



// gets all files in directory 'path'
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

		string filepath = ( path == "." ? "" : path + "/" ) + ent->d_name;
		stat(filepath.c_str(), &st);  // get file statistics

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


// find a list of #includes in the current file, and save in the files 'deps' array
int find_includes(cfile &cf) {
	smatch match;
	string s;
	fstream file;
	file.open(cf.fpath());

	while (!file.eof()) {
		getline(file, s);
		if (regex_search(s, match, includefile)) {
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


// compile single file
int compile(const cfile &cf, int& did_compile) {
	did_compile = 0;
	string objname = "bin/" + cpp_base_fname(cf) + ".o";
	bin_files += " " + objname;

	if (latest_modtime(objname) < latest_modtime(cf)) {
		did_compile = 1;
		string command = config::CC() 
			+ " -I."
			+ ( cf.path == "." ? "" : " -I" + cf.path )
			+ config::pkg_config("cflags")
			+ " -c -o " + objname 
			+ " " + cf.fpath();
		cout << command << endl;
		return system(command.c_str());
	}
	return 0;  // no error code
}


// link all files in "./bin". 'compile_count' is the number of .o files compiled in the compile step
int link_all(string outfile, int compile_count) {
	if (latest_modtime(outfile) == 0 || compile_count > 0) {
		string cmd = config::CC() 
			+ bin_files 
			+ config::pkg_config("libs") 
			+ " -o " + outfile;
		cout << cmd << endl;
		return system(cmd.c_str());
	}
	return 0;  // no error code
}



int main(int argc, char** argv) {
	currenttime = time(NULL);
	args::parse(argc, argv);

	// check if we need to print help messages. if so, do it and exit
	if (args::print_help())
		return 0;

	// check if we just need to do cleanup
	if (args::has_arg("clean") || args::has_arg("rebuild")) {
		cout << "cleaning bin files..." << endl;
		system("rm -rf bin");  // just delete bin and exit
		if (args::has_arg("clean"))
			return 0;
	}

	// load config from dmake.conf file
	config::load();

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
		cout << txt_cyan << f.fpath() << " :: " << txt_reset;
		for (auto i : f.deps)
			cout << files[i].fname << "  ";
		// cout << "]  " << (__int64_t)latest_modtime(f) << endl;
		cout << endl;
	}
	cout << endl;

	// compile all files
	int compile_count = 0;
	for (auto &f : files) {
		int err = 0, did_compile = 0;
		if (is_source(f))
			err = compile(f, did_compile);
		if (err)
			return 1;  // stop here if there was an error
		if (did_compile)
			compile_count++;
	}

	// make the main executable
	string outfile = "bin/" + config::outfile();
	int err = link_all(outfile, compile_count);
	if (err)
		return err;
	cout << endl;

	// run the executable, if required
	if (args::has_arg("run")) {
		cout << "running: " << outfile << endl;
		err = system(outfile.c_str());
	}

	// return either main.out make error, or run error
	return err;
}
