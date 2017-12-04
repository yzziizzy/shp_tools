#pragma once



#include <string>
#include <map>
#include <vector>

#include "common.h"
#include "TRE.h"


using namespace std;

class RGNElementGroup {
public:
	int polyType;
	bool has2ByteLen;
	bool direction;
	size_t streamLen;
	
	
}

class RGNPolylineSegment {
public:
	
	int polyType;
	bool has2ByteLen;
	bool direction;
	size_t streamLen;
	
	// label info
	//   TODO
	
	MapCoord latDelta;
	MapCoord lonDelta;
	
	MapCoord latAbs;
	MapCoord lonAbs;
};

class RGNSegment {
public:
	
	uint8_t* basePtr;
	uint8_t* pointsPtr;
	uint8_t* pointsEndPtr; // end pointers point to the last byte of the region.
	uint8_t* indexedPointsPtr;
	uint8_t* indexedPointsEndPtr;
	uint8_t* polylinesPtr;
	uint8_t* polylinesEndPtr;
	uint8_t* polygonsPtr;
	uint8_t* polygonsEndPtr;
	
	
	
};


class RGNFile {
public:
	
	TREFile* tre;
	
	string filePath;
	string fileName;
	string fileBaseName;
	string fileExt;
	string fileDir;
	
	
	
	uint8_t* data; 
	uint32_t dataLen; 
	
	vector<uint8_t*> segmentPtrs;
	
	
	void LoadPath(string* path);
	void ParseBuffer(uint8_t* raw, size_t rawLen);
	
};


