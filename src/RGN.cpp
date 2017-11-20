

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "common.h"
#include "bitstream.h"

#include "TRE.h"
#include "RGN.h"


// data order:
// points, indexed points, polylines, polygons




void RGNFile::LoadPath(string* path) {
	
	// messy file path string stuff
	char* rp = realpath(path->c_str(), NULL);
	filePath.assign(rp);
	char* rp2 = strdup(rp); // dirname may modify it
	fileDir.assign(dirname(rp)); 
	fileName.assign(basename(rp2)); 
	free(rp);
	free(rp2);
	
	smatch m;
	if(regex_search(fileName, m, r)) {
		fileBaseName = m[1];
		fileExt = m[2];
	}
	
	raw = readFile(filePath.c_str(), &rawLen);
	
	ParseBuffer(raw, rawLen);
	
}



void RGNFile::ParseBuffer(uint8_t* raw, size_t rawLen) {
	
	data = read32u(raw + 0x15);
	dataLen = read32u(raw + 0x19);
	
	
	vector<TRESubdivision*>::iterator it;
	
	for(it = tre->subdivisions.begin(); it != tre->subdivisions.end(); it++) {
		uint8_t* p = data + (*it)->RGNoffset;
		
		RGNSegment* s = new RGNSegment();
		s->basePtr = p;
		
		int i = 0;
		
		if((*it)->hasPoints) {
			s->pointsPtr = p + read16u(p + (2 * i));
			i++;
		}
		
		if((*it)->hasIndexedPoints) {
			s->indexedPointsPtr = p + read16u(p + (2 * i));
			i++;
		}
		
		if((*it)->polylinesPtr) {
			s->polylinesPtr = p + read16u(p + (2 * i));
			i++;
		}
		
		if((*it)->polygonsPtr) {
			s->polygonsPtr = p + read16u(p + (2 * i));
			i++;
		}
		
		
		
		
		
		
	}
	
	
}

























