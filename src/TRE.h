#pragma once

#include <string>
#include <map>
#include <vector>

#include "common.h"

using namespace std;




class TREMapLevel {
public:
	int zoomLevel;
	int bitsPerCoord;
	int subdivisions;
};

class TRESubdivision {
public:
	TREMapLevel* level;
	
	size_t RGNoffset;
	
	bool hasPoints;
	bool hasIndexedPoints;
	bool hasPolylines;
	bool hasPolygons;
	int numTypes;
	
	bool terminatingFlag;
	
	MapCoord centerLat;
	MapCoord centerLon;
	
	MapCoord width;
	MapCoord height;
	
	uint16_t nextLevelIndex; // 1-based
	
	
	TRESubdivision(uint8_t* data, bool isLowest);
};

class TREPrimOverview {
public:
	int type;
	int maxLevelPresent;
	int pointSubType;
	
	TREPrimOverview(uint8_t type, uint8_t maxLevel);
	TREPrimOverview(uint8_t type, uint8_t maxLevel, uint8_t pointSubType);
};

class TREFile {
public:
	
	string filePath;
	string fileName;
	string fileBaseName;
	string fileExt;
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
	
	
	vector<TREPrimOverview*> polylineOverview; 
	vector<TREPrimOverview*> polygonOverview; 
	vector<TREPrimOverview*> pointOverview; 
	
	
	uint16_t headerLen;
	
	void LoadPath(string* path);
	void ParseBuffer(uint8_t* buff, size_t blen);
	
};










