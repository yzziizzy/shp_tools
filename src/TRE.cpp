#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <regex>
#include <map>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>

#include "TRE.h"


using namespace std;



#define PRINT(x) cout << #x ": " << (x) << endl;

void TREFile::LoadPath(string* path) {
	
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
	
	raw = readFile(filePath.c_str(), &rawLen);
	
	ParseBuffer(raw, rawLen);
}


void TREFile::ParseBuffer(uint8_t* raw, size_t rawLen) {
	
	// general header
	subHeader = raw + 0x15;
	
	headerLen = read16u(raw);
	data = raw + headerLen;
	cout << "header Len: " << headerLen << endl;
	
	if(raw[0xd]) {
		cout << "WARNING: map level data is encrypted.\n"; 
	}
	
	// TRE subheader
	
	// absolute map boundaries
	bounds.N.n = read24(raw + 0x15);
	bounds.E.n = read24(raw + 0x18);
	bounds.S.n = read24(raw + 0x1b);
	bounds.W.n = read24(raw + 0x1e);
	bounds.N.d = coordFrom24(bounds.N.n);
	bounds.E.d = coordFrom24(bounds.E.n);
	bounds.S.d = coordFrom24(bounds.S.n);
	bounds.W.d = coordFrom24(bounds.W.n);
	
	long off = read32u(raw + 0x21); 
	mapLevelsPtr = raw + off;
	mapLevelsLen = read32u(raw + 0x25);
	cout << "mapLevelsPtr: " << off << " (" << mapLevelsLen << ")" << endl;
	
	off = read32u(raw + 0x29); 
	subdivisionsPtr = raw + off;
	subdivisionsLen = read32u(raw + 0x2d);
	cout << "subdivisionsPtr: " << off << " (" << subdivisionsLen << ")" << endl;
	
	off = read32u(raw + 0x31); 
	copyrightPtr = raw + off;
	copyrightLen = read32u(raw + 0x35);
	copyrightRecSize = read16u(raw + 0x39);
	cout << "copyrightPtr: " << off << " (" << copyrightLen << " / " << copyrightRecSize << ")" << endl;
	
	// geom sections
	off = read32u(raw + 0x4a); 
	polylineOverviewPtr = raw + off;
	polylineOverviewLen = read32u(raw + 0x4e);
	polylineOverviewRecSize = read16u(raw + 0x52);
	cout << "polylineOverviewPtr: " << off << " (" << polylineOverviewLen << " / " << polylineOverviewRecSize << ")" << endl;
	
	off = read32u(raw + 0x58); 
	polygonOverviewPtr = raw + off;
	polygonOverviewLen = read32u(raw + 0x5c);
	polygonOverviewRecSize = read16u(raw + 0x60);
	cout << "polygonOverviewPtr: " << off << " (" << polygonOverviewLen << " / " << polygonOverviewRecSize << ")" << endl;
	
	off = read32u(raw + 0x66); 
	pointOverviewPtr = raw + off;
	pointOverviewLen = read32u(raw + 0x6a);
	pointOverviewRecSize = read16u(raw + 0x6e);
	cout << "pointOverviewPtr: " << off << " (" << pointOverviewLen << " / " << pointOverviewRecSize << ")" << endl;
	
	
	int subsCount = 0;
	int subsBeforeLowest = 0;
	
	// parse map levels
	uint8_t* p = mapLevelsPtr;
	for(int i = 0; i < mapLevelsLen / 4; i++) {
		TREMapLevel* ml = new TREMapLevel();
		unsigned char x = *p;
		ml->zoomLevel = (x & 0x7);
		ml->bitsPerCoord = p[1];
		ml->subdivisions = read16u(p + 2);
		
		subsCount += ml->subdivisions;
		if(ml->zoomLevel > 0) subsBeforeLowest += ml->subdivisions;
		
		cout << "Map Level " << ml->zoomLevel << ": \n  bitsPerCoord: " << ml->bitsPerCoord
			<< "\n  subdivisions: " << ml->subdivisions << endl;  
		
		mapLevels.push_back(ml);
		
		p += 4;
	}
	
	cout << "\n\n";
	
	// parse subdivisions
	p = subdivisionsPtr;
	//cout << "subs before lowest: " << subsBeforeLowest << endl;
	
	for(int i = 0; i < subsCount; i++) {
		bool onLowest = i > subsBeforeLowest;
		TRESubdivision* sd = new TRESubdivision(p, onLowest);
		
		subdivisions.push_back(sd);
		
// 		if(i < 2) {
// 			
// 		cout << "Subdivision " << i << ":\n  dims: " << sd->width.n << " x " << sd->height.n << endl;
// 		cout << "  RGN offset: " << sd->RGNoffset << endl; 
// 		cout << "  terminating: " << sd->terminatingFlag << endl;
// 				cout << "   RGN offset: " << bitset<24>(*((uint32_t*)(p+0))) << endl;
// 				cout << "   flags: " << bitset<8>(*((uint32_t*)(p+3))) << endl;
// 				cout << "   lon: " << bitset<24>(*((uint32_t*)(p+4))) << endl;
// 				cout << "   lat: " << bitset<24>(*((uint32_t*)(p+7))) << endl;
// 				cout << "   w: " << bitset<16>(*((uint16_t*)(p+10))) << endl;
// 				cout << "   h: " << bitset<16>(*((uint16_t*)(p+12))) << endl;
// 		}
//		else break;
		p += (onLowest ? 14 : 16);
	}
	
	
	p = polylineOverviewPtr;
	while(p < polylineOverviewPtr + polylineOverviewLen) {
		polylineOverview.push_back(new TREPrimOverview(p[0], p[1]));
		p += polylineOverviewRecSize;
	}
	
	p = polygonOverviewPtr;
	while(p < polygonOverviewPtr + polygonOverviewLen) {
		polygonOverview.push_back(new TREPrimOverview(p[0], p[1]));
		p += polygonOverviewRecSize;
	}

	p = pointOverviewPtr;
	while(p < pointOverviewPtr + pointOverviewLen) {
		pointOverview.push_back(new TREPrimOverview(p[0], p[1], p[2]));
		p += pointOverviewRecSize;
	}
	
};



TREPrimOverview::TREPrimOverview(uint8_t type, uint8_t maxLevel) {
	this->type = type;
	this->maxLevelPresent = maxLevel;
};
TREPrimOverview::TREPrimOverview(uint8_t type, uint8_t maxLevel, uint8_t pointSubType) {
	this->type = type;
	this->maxLevelPresent = maxLevel;
	this->pointSubType = pointSubType;
};


TRESubdivision::TRESubdivision(uint8_t* data, bool isLowest) {
	RGNoffset = read24u(data);
	

	
	hasPoints = 0x01 & data[3];
	hasIndexedPoints = 0x02 & data[3];
	hasPolylines = 0x04 & data[3];
	hasPolygons = 0x08 & data[3];
	
	centerLon.read24(data + 4);
	centerLat.read24(data + 7);
	
	terminatingFlag = *(data + 11) & (0x80); 
	
	// BUG: not clear if width is scaled on 15 or 16 bits; it appears to be on 16
	uint16_t x = *((uint16_t*)(data + 10)) & 0x7fff;
	width.fromN(x, 0, 16);
	height.readN(data + 12, 0, 16);
	
	// conditionally read the next subdivision level
	if(!isLowest) {
		nextLevelIndex = read16u(data + 14);
	}
}




