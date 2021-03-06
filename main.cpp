#include <iostream>
#include <fstream>
// #include <regex>
#include <sstream>
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

const string txt_cyan = "\e[0;36m";
const string txt_reset = "\e[0m";

// string CC = "clang++ -std=c++11 -stdlib=libc++  -Wall";  // sensible default
vector<cfile> files;
string bin_files;
int currenttime = 0;



// --- helpers ---

// const regex cppfile(".+\\.(cpp|hpp|c|h)");
// const regex cppsourcefile("(.+)\\.(cpp|c)");
// const regex includefile("\\s*#include\\s+\"(.+)\"");
// const regex fileextension("^.+\\.(.+)$");

// helper - return extension substring of file name
// static string file_extension(const cfile &cf) {
// 	smatch match;
// 	if (regex_search(cf.fname, match, fileextension))
// 		return tolower(match[1]);
// 	return "";
// }
// is c++ source/header file
// static int is_cpp(const string& fname) {
// 	if (regex_match(fname, cppfile))
// 		return 1;
// 	return 0;
// }
// helper - true for "*.cpp/*.c"
// static int is_cpp_source(const cfile &cf) {
// 	if (regex_match(cf.fname, cppsourcefile))
// 		return 1;
// 	return 0;
// }
// helper - returns (fname).cpp from filename
// static string cpp_base_fname(const cfile &cf) {
// 	smatch match;
// 	if (regex_search(cf.fname, match, cppsourcefile))
// 		return match[1];
// 	return "";
// }
// check for #include line
// static string include_fname(const string& line) {
// 	smatch match;
// 	if (regex_search(line, match, includefile))
// 		return match[1];
// 	return "";
// }

// helper - return extension substring of file name
static string file_extension(const string& fname) {
	int ex = fname.find_last_of('.');
	if (ex != string::npos)
		return tolower(fname.substr(ex+1));
	return "";
}
// helper - true for "*.cpp/*.c"
static int is_cpp_source(const string& fname) {
	string ex = file_extension(fname);
	if (ex == "cpp")  return 1;
	if (ex == "c"  )  return 1;
	return 0;
}
// is c++ source or header file
static int is_cpp(const string& fname) {
	string ex = file_extension(fname);
	if (ex == "hpp")  return 1;
	if (ex == "h"  )  return 1;
	return is_cpp_source(fname);
}
// helper - returns (fname).cpp from filename
static string cpp_base_fname(const string& fname) {
	string ex = file_extension(fname);
	if (is_cpp_source(fname))
		return fname.substr(0, fname.find_last_of('.'));
	return "";
}
// check for #include line
static string include_fname(const string& line) {
	stringstream ss(line);
	string s;
	ss >> s >> ws;
	if (tolower(s) != "#include")  return "";  // #include line
	if (ss.peek() != '"')  return "";  // check for string start
	s = "",  ss.get();  // prepare loop
	while (ss.peek() != '"') {
		if (ss.peek() == EOF)  return "";
		s += ss.get();
	}
	return s;
}
// helper - latest modified file, uses file info
//   uses crude MAX_DEPTH to prevent possible recursion problems
static double latest_modtime(const cfile &cf, int depth = 0) {
	static const int MAX_DEPTH = 10;
	double mtime = cf.mtime;
	if (depth > MAX_DEPTH)
		return mtime;
	for (auto i : cf.deps) {
		int newtime = latest_modtime(files[i], depth+1);
		if (newtime > mtime)
			mtime = newtime;
	}
	return mtime;
}
// helper - latest modified file. takes a single file (anything, obj file)
static double latest_modtime(const string &fpath) {
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
			if (is_cpp(ent->d_name))
				files.push_back(cfile(
					ent->d_name,
					path,
					st.st_mtime
				));
		}
	}

	closedir(dir);
	return 0;
}


// find a list of #includes in the current file, and save in the files 'deps' array
int find_includes(cfile &cf) {
	string s, fname;
	fstream file;
	file.open(cf.fpath());

	while (!file.eof()) {
		getline(file, s);
		fname = include_fname(s);
		// printf(":: [%s]  %s\n", fname.c_str(), s.c_str());
		if (fname.length()) {
			// find base filename, in case we are searching through sub-directories
			int pos = fname.find_last_of('/');
			if (pos != string::npos)
				fname = fname.substr(pos+1);
			// cout << fname << "  " << pos << endl;
			// add file indexes to the list
			for (int i = 0; i < files.size(); i++) {
				if (files[i].fname == fname)
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
	string objname = "bin/" + cpp_base_fname(cf.fname) + ".o";
	bin_files += " " + objname;

	if (latest_modtime(objname) < latest_modtime(cf)) {
		did_compile = 1;
		string command = config::CC( file_extension(cf.fname) ) 
			+ " " + cf.fpath()
			+ config::ccflags()
			+ " -I."
			+ ( cf.path == "." ? "" : " -I" + cf.path )
			+ config::pkg_config("cflags")
			+ " -c -o " + objname;
		cout << command << endl;
		// cout << file_extension(cf.fname) << endl;
		return system(command.c_str());
	}
	return 0;  // no error code
}


// link all files in "./bin". 'compile_count' is the number of .o files compiled in the compile step
int link_all(string outfile, int compile_count) {
	if (latest_modtime(outfile) == 0 || compile_count > 0) {
		string cmd = config::CC("cpp") 
			+ bin_files 
			+ config::ldflags()
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
	if (args::print_help())  return 0;
	if (args::print_version())  return 0;

	// check if we just need to do cleanup
	if (args::has_arg("clean") || args::has_arg("rebuild") || args::has_switch("b")) {
		cout << "cleaning bin files..." << endl;
		// just delete bin and exit
		int err = ( platform::OS_STRING == "windows" 
			? system("if exist bin rmdir /Q/S bin")
			: system("rm -rf bin") );
		if (err) {
			cerr << "system error: " << err << endl;
			return err;
		}
		if (args::has_arg("clean"))
			return 0;
	}	

	// make bin directory
	{
		int err = ( platform::OS_STRING == "windows" 
			? system("if not exist bin mkdir bin")
			: system("mkdir -p bin") );
		if (err) {
			cerr << "system error: " << err << endl;
			return err;
		}
	}

	config::load();  // load config from dmake.conf file
	filelist(".");   // get all base files

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
		cout << "  " << txt_cyan << f.fpath() << " :: " << txt_reset;
		for (auto i : f.deps)
			cout << files[i].fname << "  ";
		// cout << "]  " << (__int64_t)latest_modtime(f) << endl;
		cout << endl;
	}

	// compile all files
	int compile_count = 0;
	for (auto &f : files) {
		int err = 0, did_compile = 0;
		if (is_cpp_source(f.fname))
			err = compile(f, did_compile);
		if (err)
			return 1;  // stop here if there was an error. compiler reports errors
		if (did_compile)
			compile_count++;
	}

	// make the main executable
	string outfile = "bin/" + config::outfile();
	{
		int err = link_all(outfile, compile_count);
		if (err)
			return err;  // compiler reports errors
	}

	// run the executable, if required
	if (args::has_arg("run")) {
		if (platform::OS_STRING == "windows")
			outfile[3] = '\\';  // this will fail on windows with a forward slash for some reason
		cout << txt_cyan << "running: " << outfile << endl
			<< "---" << txt_reset << endl;
		int err = system(outfile.c_str());
		if (err) {
			cerr << "system error: " << err << endl;
			return err;
		}
	}

	// all ok
	return 0;
}
