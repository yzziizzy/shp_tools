

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
#include <math.h>

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
		cout << "rgnOffset: 0x" << hex << (uint64_t)(*it)->RGNoffset << dec << endl;  
		if(si > 200) return;
		cout << "segment " << si++ << " ["; 
		
		int i = 0;
		
		uint8_t** lastEnd = NULL;
		
		// all coordinates need to be adjusted with this
		int levelShift = 24 - sub->level->bitsPerCoord;
		
		// ths first pointer is not stored. the first data set is located at the end of the pointers
		if(sub->hasPoints) {
			if(i) s->pointsPtr = p + read16u(p + (2 * i));
			else s->pointsPtr = p + ((sub->numTypes - 1) * 2);
			i++;
			
			lastEnd = &s->pointsEndPtr;
			
			cout << "points ";
		}
		
		if(sub->hasIndexedPoints) {
			if(i) s->indexedPointsPtr = p + read16u(p + (2 * i));
			else s->indexedPointsPtr = p + ((sub->numTypes - 1) * 2);
			i++;
			
			if(lastEnd) *lastEnd = s->indexedPointsPtr - 1;
			lastEnd = &s->indexedPointsEndPtr;
			
			cout << "indexedPoints ";
		}
		
		if(sub->hasPolylines) {
			if(i) s->polylinesPtr = p + read16u(p + (2 * i));
			else s->polylinesPtr = p + ((sub->numTypes - 1)* 2);
			i++;
			
			if(lastEnd) *lastEnd = s->polylinesPtr - 1;
			lastEnd = &s->polylinesEndPtr;
			
			cout << "polylines ";
		}
		
		if(sub->hasPolygons) {
			if(i) s->polygonsPtr = p + read16u(p + (2 * i));
			else s->polygonsPtr = p + ((sub->numTypes - 1) * 2);
			i++;
			
			if(lastEnd) *lastEnd = s->polygonsPtr - 1;
			lastEnd = &s->polygonsEndPtr;
			
			cout << "polygons";
		}
		
		cout << "]\n";
		
		// nothing more to do
		if(i == 0) continue;
		
		// the last segment ends where the next begins
		if(lastEnd) {
			// BUG: check that this logic is right
			if(*(it + 1)) {
				*lastEnd = data + (*(it + 1))->RGNoffset - 1;
			}
			else {
				*lastEnd = raw + rawLen - 1;
			}
		}
		
		
		p += 2 * i;
		
		Bitstream b;
		
		if(sub->hasPolylines) {
			b.Init(s->polylinesPtr, 0);
			
			int type = b.ReadN(6);
			int direction = b.ReadN(1);
			int twoByteLen = b.ReadN(1);

			
			
			// skip label info for now
			uint32_t LBLoffset = b.ReadN(22);
			bool extraBit = b.ReadN(1);
			bool dataInNET = b.ReadN(1);
			
			
			uint16_t lonDelta = b.ReadN(16); 
			uint16_t latDelta = b.ReadN(16); 
			
			int streamLen = b.ReadN(twoByteLen ? 16 : 8);
			
			
			// bitstream_info byte
			uint16_t lonBits = b.ReadN(4);
			uint16_t latBits = b.ReadN(4);
			
			bool lonSignConsistent = b.ReadN(1);
			if(lonSignConsistent) {
				bool lonSign = b.ReadN(1); // 0 = +, 1 = -
			}
			
			bool latSignConsistent = b.ReadN(1);
			if(latSignConsistent) {
				bool latSign = b.ReadN(1);
			}
			
			int lonReadBits = 2 + lonBits + !lonSignConsistent + extraBit;
			int latReadBits = 2 + latBits + !latSignConsistent + extraBit;
			
			
			
			// actual primitive data here
			int64_t lastLon = 0;
			int64_t lastLat = 0;
			
			int j = 0;
			while(b.Tell() < s->polylinesEndPtr + (int64_t)ceil((float)(lonReadBits + latReadBits) / 8.0)) {
				int64_t lon1 = b.ReadN(lonReadBits);
				int64_t lat1 = b.ReadN(latReadBits);
				
				int64_t lon = (lon1 + lastLon + lonDelta) << levelShift; 
				int64_t lat = (lat1 + lastLat + latDelta) << levelShift; 
				
				double dlon = coordFromN(lon, sub->level->bitsPerCoord);
				double dlat = coordFromN(lat, sub->level->bitsPerCoord);
				
				dlon += sub->centerLon.d;
				dlat += sub->centerLat.d;
				
				cout << dec;
				cout << "lat/lon: [" << lat << ", " << lon << "]  ";
				cout << "(" << dlat << ", " << dlon << ")" << endl;
				
				
				lastLon = lon1;
				lastLat = lat1;
				
				j++;
			}
			
			cout << "  " << j << " polyline nodes found\n";
			
		}
		
		
		
	}
	
	
}

























