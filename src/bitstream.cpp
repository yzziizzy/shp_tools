

#include <bitset>
#include <iostream>

#include "bitstream.h"


using namespace std;


void Bitstream::Init(uint8_t* data, int bitOffset) {
	base = cursor = data;
	baseOffsetBits = bitOffset;
	totalCursorBits = 0;
}

void Bitstream::Seek(int bits) {
	
	totalCursorBits += bits;
	if(totalCursorBits < 0) totalCursorBits = 0;
	
	cursor = base + (totalCursorBits / 8);
}


#define PB(y ,x, n) cout << #y ": " << bitset<n>(x) << endl

// max 32
uint32_t Bitstream::PeekN(int bits) {
	if(bits > 32) {
		cout << "Bitstream::PeekN - requsted bits > 32 \n";
		exit(69);
	}
	
	int ob = (totalCursorBits % 8) + baseOffsetBits;
	cout << "ob: " << ob << " tCb: " << totalCursorBits << endl;
	uint64_t raw = *((uint64_t*)cursor);
	
	PB(1, raw, 64);

	raw >>= (ob); // max offset it 8 bits, for now; max data len is 24 per file limits
	PB(2, raw, 64);
	
	raw <<= ((64 - bits)); // max offset it 8 bits, for now; max data len is 24 per file limits
	PB(3, raw, 64);
	// move data to the right
	// move data to the right
	raw >>= (64 - bits);
	PB(4, raw, 64);
	return raw;
}


uint32_t Bitstream::ReadN(int bits) {
	uint32_t n = PeekN(bits);
	Seek(bits);
	return n;
}




