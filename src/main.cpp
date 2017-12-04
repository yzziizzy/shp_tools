#include <iostream>
#include <string>
#include <vector>
#include <regex>


#include "shp.h"


// yup. don't care. fuck off.
using namespace std;




class ProgramOpts {
public:
	vector<string> filenames;
	
};


class Context {
public:
	vector<string> shpFileNames;
	
	
	void AddFiles(vector<string>* paths);
	
	vector<SHPInfo*> shpfiles;
	
};



void Context::AddFiles(vector<string>* paths) {
	
	static regex r("^(.*)\\.([^.]*)$");
	
	
	for(vector<string>::iterator it = paths->begin(); it != paths->end(); it++) {
		smatch m;
		if(regex_search(*it, m, r)) {
		
			cout << " base: " << m[1] << ", ext: " << m [2] << endl;
		}
		else {
			cout << "invalid file name: " << *it << endl;
		}
	}
	
	
	
}




int main(int argc, char* argv[]) {
	ProgramOpts opts;
	Context c;
	
	for(int i = 1; i < argc; i++) {
		if(argv[i][0] == '-') {
			if(argv[i][1] == '-') {
				// long form
				
			}
			else { // short form
				
			}
		}
		else { // a file
			opts.filenames.push_back(string(argv[i]));
		}
		
		
		
	}
	
	c.AddFiles(&opts.filenames);
	
	
	return 0;
}
