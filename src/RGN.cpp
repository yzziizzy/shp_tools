

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdio.h>
#include <string.h>
#include <regex>
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
	
	static regex r("^(.*)\\.([^.]*)$");
	
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
	
	size_t rawLen;
	uint8_t* raw = readFile(filePath.c_str(), &rawLen);
	
	// try to find the TRE file first
	
	tre = new TREFile();
	string tp = fileDir + "/" + fileBaseName + ".TRE"; 
	cout << "Attempting to parse TRE file first: " << tp << endl;
	tre->LoadPath(&tp);
	
	ParseBuffer(raw, rawLen);
	
}



void RGNFile::ParseBuffer(uint8_t* raw, size_t rawLen) {
	
	data = raw + read32u(raw + 0x15);
	dataLen = read32u(raw + 0x19);
	
	
	vector<TRESubdivision*>::iterator it;
	
	int si = 0;
	for(it = tre->subdivisions.begin(); it != tre->subdivisions.end(); it++) {
		uint8_t* p = data + (*it)->RGNoffset;
		TRESubdivision* sub = *it;
		
		RGNSegment* s = new RGNSegment();
		s->basePtr = p;
		
		cout << "segment " << si++ << endl; 
		
		int i = 0;
		
		// ths first pointer is not stored. the first data set is located at the end of the pointers
		if(sub->hasPoints) {
			if(i) s->pointsPtr = p + read16u(p + (2 * i));
			else s->pointsPtr = p + ((sub->numTypes - 1) * 2);
			i++;
		}
		
		if(sub->hasIndexedPoints) {
			if(i) s->indexedPointsPtr = p + read16u(p + (2 * i));
			else s->indexedPointsPtr = p + ((sub->numTypes - 1) * 2);
			i++;
		}
		
		if(sub->hasPolylines) {
			if(i) s->polylinesPtr = p + read16u(p + (2 * i));
			else s->polylinesPtr = p + ((sub->numTypes - 1)* 2);
			i++;
		}
		
		if(sub->hasPolygons) {
			if(i) s->polygonsPtr = p + read16u(p + (2 * i));
			else s->polygonsPtr = p + ((sub->numTypes - 1) * 2);
			i++;
		}
		
		
		p += 2 * i;
		
		Bitstream b;
		
		if(sub->hasPolylines) {
			b.Init(s->polylinesPtr, 0);
			
			int type = b.ReadN(6);
			bool direction = b.ReadN(1);
			bool twoByteLen = b.ReadN(1);
			
			// skip label info for now
			uint32_t LBLoffset = b.ReadN(22);
			bool extraBit = b.ReadN(1);
			bool dataInNET = b.ReadN(1);
			
			uint16_t lonDelta = b.ReadN(16); 
			uint16_t latDelta = b.ReadN(16); 
			
			int streamLen = b.ReadN(twoByteLen ? 16 : 8);
			
			// bitstream_info byte
			uint8_t lonBits = b.ReadN(4);
			uint8_t latBits = b.ReadN(4);
			
			// the data bitstream starts now
			
			
		}
		
		
		
	}
	
	
}

























