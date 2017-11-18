#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <map>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>

#include "TRE.h"


using namespace std;

unsigned char* readFile(const char* path, size_t* srcLen) {
	
	size_t fsize;
	unsigned char* contents;
	FILE* f;
	
	
	f = fopen(path, "rb");
	if(!f) {
		fprintf(stderr, "Could not open file \"%s\"\n", path);
		return NULL;
	}
	
	fseek(f, 0, SEEK_END);
	fsize = ftell(f);
	rewind(f);
	
	contents = (unsigned char*)malloc(fsize);
	
	fread(contents, sizeof(unsigned char), fsize, f);
	
	fclose(f);
	
	if(srcLen) *srcLen = fsize;
	
	return contents;
}


uint16_t read16u(uint8_t* b) {
	return *((uint16_t*)b);
}
uint32_t read32u(uint8_t* b) {
	return *((uint32_t*)b);
}

int32_t read24(uint8_t* b) {
	return (  
		  (b[0] << 24)
		| (b[1] << 16)
		| (b[2] << 8)
	) >> 8; 
}

uint32_t read24u(uint8_t* b) {
	uint32_t p = b[0];
	uint32_t q = b[1];
	uint32_t r = b[2];
	
	return ((p << 24) | (q << 16) | (r << 8)) >> 8; 
}

double coordFrom24(int32_t i) {
	return (double)i * (360.0 / (double)(1 << 24));
} 

void MapCoord::read24(uint8_t* data) {
	n = ::read24(data);
	d = coordFrom24(n);
	sourceBits = 24;
}

// BUG: this function, probably
void MapCoord::readN(uint8_t* data, int offsetBits, int dataBits, bool isSigned = false) {
	fromN(*((uint32_t*)data), offsetBits, dataBits, isSigned);
}
	
void MapCoord::fromN(uint32_t raw, int offsetBits, int dataBits, bool isSigned = false) {
	sourceBits = dataBits;
	this->isSigned = isSigned;
	
	cout << "-------------\n";
	// remove offset.
	cout << bitset<32>(raw) << endl;
	
	raw <<= ((31-dataBits) + offsetBits ); // max offset it 8 bits, for now; max data len is 24 per file limits
	
	cout << bitset<32>(raw) << endl;
	
	bool sign = (1 << 23) & raw;
	
	// move data to the right
	raw >>= (31 - dataBits) + offsetBits;
	
	cout << bitset<32>(raw) << endl;
	
	// the two shifts should have cleared all non-data bits
	
	n = raw;
	
	cout<<n<< endl;;
	
	double unit = 360.0 / (double)(1 << (dataBits));
	d = unit * n;
	
	cout << d << endl;
	
	cout << "^^^^^^^^^^^^\n";
	if(isSigned && sign) {
		d = -fabs(d);
		n = -abs(n);
	}
}



#define PRINT(x) cout << #x ": " << (x) << endl;


void TREFile::LoadPath(string* path) {
	
	// messy file path string stuff
	char* rp = realpath(path->c_str(), NULL);
	filePath.assign(rp);
	char* rp2 = strdup(rp); // dirname may modify it
	fileDir.assign(dirname(rp)); 
	fileName.assign(basename(rp2)); 
	free(rp);
	free(rp2);
	
	// general header
	raw = readFile(filePath.c_str(), &rawLen);
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
	cout << "subs before lowest: " << subsBeforeLowest << endl;
	
	for(int i = 0; i < subsCount; i++) {
		bool onLowest = i > subsBeforeLowest;
		TRESubdivision* sd = new TRESubdivision(p, onLowest);
		
		subdivisions.push_back(sd);
		
		if(i < 2) {
			
		cout << "Subdivision " << i << ":\n  dims: " << sd->width.n << " x " << sd->height.n << endl;
		cout << "  RGN offset: " << sd->RGNoffset << endl; 
		cout << "  terminating: " << sd->terminatingFlag << endl;
				cout << "   RGN offset: " << bitset<24>(*((uint32_t*)(p+0))) << endl;
				cout << "   flags: " << bitset<8>(*((uint32_t*)(p+3))) << endl;
				cout << "   lon: " << bitset<24>(*((uint32_t*)(p+4))) << endl;
				cout << "   lat: " << bitset<24>(*((uint32_t*)(p+7))) << endl;
				cout << "   w: " << bitset<16>(*((uint16_t*)(p+10))) << endl;
				cout << "   h: " << bitset<16>(*((uint16_t*)(p+12))) << endl;
		}
		else break;
		p += (onLowest ? 14 : 16);
	}
	
	
}




	
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

