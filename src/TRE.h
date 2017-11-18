
#include <string>
#include <map>
#include <vector>

using namespace std;


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


class TREMapLevel {
public:
	int zoomLevel;
	int bitsPerCoord;
	int subdivisions;
};

class TRESubdivision {
public:
	
	size_t RGNoffset;
	
	bool hasPoints;
	bool hasIndexedPoints;
	bool hasPolylines;
	bool hasPolygons;
	
	bool terminatingFlag;
	
	MapCoord centerLat;
	MapCoord centerLon;
	
	MapCoord width;
	MapCoord height;
	
	uint16_t nextLevelIndex; // 1-based
	
	
	TRESubdivision(uint8_t* data, bool isLowest);
};

class TREFile {
public:
	
	string filePath;
	string fileName;
	string fileDir;
	
	uint8_t* data;
	uint8_t* subHeader;
	uint8_t* raw;
	size_t rawLen;
	
	uint8_t* mapLevelsPtr;
	size_t mapLevelsLen;
	uint8_t* subdivisionsPtr;
	size_t subdivisionsLen;
	uint8_t* copyrightPtr;
	size_t copyrightLen;
	size_t copyrightRecSize;
	uint8_t* polylineOverviewPtr;
	size_t polylineOverviewLen;
	size_t polylineOverviewRecSize;
	uint8_t* polygonOverviewPtr;
	size_t polygonOverviewLen;
	size_t polygonOverviewRecSize;
	uint8_t* pointOverviewPtr;
	size_t pointOverviewLen;
	size_t pointOverviewRecSize;
	
	string mapDescriptor;
	
	struct {
		MapCoord N, E, S, W;
	} bounds;
	
	vector<TREMapLevel*> mapLevels;
	vector<TRESubdivision*> subdivisions;
	
	
	
	
	
	uint16_t headerLen;
	
	void LoadPath(string* path);
	
};
