
#include <iostream>
#include <string>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "TRE.h"








int main(int argc, char* argv[]) {
	TREFile tre;
	char dump = 0;
	
	for(int i = 1; i < argc; i++) {
		if(argv[i][0] == '-') {
			if(argv[i][1] == '-') {
				// long form
				// -x --dexor
				
				
			}
			else { // short form
				
			}
		}
		else { // a file
			string s = argv[i];
			
			tre.LoadPath(&s);
			
		}
	}


	
	return 0;
}










