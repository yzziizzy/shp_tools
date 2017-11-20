


#include "bitstream.h"





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


// max 32
uint32_t Bitstream::PeekN(int bits) {
	if(bits > 32) {
		cout << "Bitstream::PeekN - requsted bits > 32 \n";
		exit(69);
	}
	
	int ob = (totalCursorBits % 8) + baseOffsetBits;
	
	uint64_t raw = *((uint64_t*)cursor);
	
	raw <<= ((63 - bits) + ob); // max offset it 8 bits, for now; max data len is 24 per file limits
	
	// move data to the right
	raw >>= (63 - bits) + ob;
	
	return raw;
}


uint32_t Bitstream::ReadN(int bits) {
	uint32_t n = PeekN(bits);
	Seek(bits);
	return n;
}




