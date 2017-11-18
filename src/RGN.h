
#include <string>
#include <map>
#include <vector>

using namespace std;

class RGNSegment {
public:
	
	uint8_t* pointsPtr;
	uint8_t* indexedPointsPtr;
	uint8_t* polylinesPtr;
	uint8_t* polygonsPtr;
	
	
	
}


class RGNFile {
public:
	
	uint8_t* data; 
	uint32_t dataLen; 
	
	vector<uint8_t*> segmentPtrs;
	
	
	
	
}


