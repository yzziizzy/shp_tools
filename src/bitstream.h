
#pragma once


#include <cstdint>

class Bitstream {
public:
	
	uint8_t* base;
	uint8_t* cursor;
	
	uint64_t baseOffsetBits; // how many bits from base before the data starts
	uint64_t totalCursorBits; // how many bits from base+baseOffset to the cursor bit
	
	void Init(uint8_t* data, int bitOffset);
	void Seek(int bits);
	
	uint32_t PeekN(int bits);
	uint32_t ReadN(int bits);
};











