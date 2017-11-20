


#include <iostream>
#include <string>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>

#include "common.h"


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
	
//	cout << "-------------\n";
	// remove offset.
//	cout << bitset<32>(raw) << endl;
	
	raw <<= ((31-dataBits) + offsetBits ); // max offset it 8 bits, for now; max data len is 24 per file limits
	
//	cout << bitset<32>(raw) << endl;
	
	bool sign = (1 << 23) & raw;
	
	// move data to the right
	raw >>= (31 - dataBits) + offsetBits;
	
//	cout << bitset<32>(raw) << endl;
	
	// the two shifts should have cleared all non-data bits
	
	n = raw;
	
//	cout<<n<< endl;;
	
	double unit = 360.0 / (double)(1 << (dataBits));
	d = unit * n;
	
//	cout << d << endl;
	
//	cout << "^^^^^^^^^^^^\n";
	if(isSigned && sign) {
		d = -fabs(d);
		n = -abs(n);
	}
}








