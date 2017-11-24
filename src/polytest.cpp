

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

using namespace std;

typedef uint8_t u8;



// u8 data[] = { 0x05, 0x40, 0x07, 0x00, 0xbc, 0x01, 0x85, 0x00, 0x03, 0x57, 0x6d, 0x12, 0x0a };
u8 data[] = { 0x05, 0x40, 0x07, 0x00, 0xbc, 0x01, 0x85, 0x00, 0x03, 0x57, 0x6d, 0x12, 0x0a };



#define P(x) cout << #x ": " << x << endl
#define Px(x) cout << #x ": " << hex << x << endl



int main(int argc, char* argv[]) {
	
	Bitstream b;
	
	b.Init(data, 0);
			
	int type = b.ReadN(6);
	P(type);
	int direction = b.ReadN(1);
	P(direction);
	int twoByteLen = b.ReadN(1);
	P(twoByteLen);

	
	
	// skip label info for now
	uint32_t LBLoffset = b.ReadN(22);
	Px(LBLoffset);
	bool extraBit = b.ReadN(1);
	P(extraBit);
	bool dataInNET = b.ReadN(1);
	
	P(dataInNET);
	
	uint16_t lonDelta = b.ReadN(16); 
	Px(lonDelta);
	uint16_t latDelta = b.ReadN(16); 
	
	Px(latDelta);
	
	int streamLen = b.ReadN(twoByteLen ? 16 : 8);
	
	P(streamLen);
	
	// bitstream_info byte
	uint8_t lonBits = b.ReadN(4);
	P(lonBits);
	uint8_t latBits = b.ReadN(4);

	P(latBits);
	
	return 0;
}

