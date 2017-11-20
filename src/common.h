#pragma once




unsigned char* readFile(const char* path, size_t* srcLen);



class MapCoord {
public:
	int32_t n;
	double d;
	
	int sourceBits; 
	bool isSigned;
	
	void read24(uint8_t* data);
	void readN(uint8_t* data, int offsetBits, int dataBits, bool isSigned);
	void fromN(uint32_t raw, int offsetBits, int dataBits, bool isSigned);
};





static inline uint16_t read16u(uint8_t* b) {
	return *((uint16_t*)b);
}
static inline uint32_t read32u(uint8_t* b) {
	return *((uint32_t*)b);
}

static inline int32_t read24(uint8_t* b) {
	return (  
		  (b[0] << 24)
		| (b[1] << 16)
		| (b[2] << 8)
	) >> 8; 
}

static inline uint32_t read24u(uint8_t* b) {
	uint32_t p = b[0];
	uint32_t q = b[1];
	uint32_t r = b[2];
	
	return ((p << 24) | (q << 16) | (r << 8)) >> 8; 
}

static inline double coordFrom24(int32_t i) {
	return (double)i * (360.0 / (double)(1 << 24));
} 



