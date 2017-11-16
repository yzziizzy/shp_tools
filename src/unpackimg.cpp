#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>

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


int writeFile(const char* path, void* data, size_t len) {
	
	size_t fsize;
	char* contents;
	FILE* f;
	
	
	f = fopen(path, "w");
	if(!f) {
		fprintf(stderr, "Could not open file \"%s\"\n", path);
		return 1;
	}
	
	
	fwrite(data, sizeof(char), len, f);
	
	fclose(f);
	
	return 0;
}


class IMGHeader {
public:
	
	size_t blockSize;
	
	size_t firstSubFileOffset;
	uint16_t blockSequenceNumbers[480/2];
	
	unsigned char* fatTable;
	size_t fatSize;
	
	void Parse(unsigned char* data, size_t len);
};


class IMGSubFile;

class IMGFatBlock {
public:
	unsigned char* start;
	int blockIndex;
	uint16_t* blockIndices;
	
	
	string name;
	string type;
	
	uint32_t fileSize;
	IMGSubFile* file;
	
	uint16_t subFileIndex;
	
	int numBlocks;
	IMGFatBlock* firstBlock;
	IMGFatBlock* nextBlock;
	
	uint16_t blockSequenceNumbers[480/2];
	
	
	IMGFatBlock(unsigned char* data, size_t dlen);
}



class IMGSubFile {
public:
	size_t size;
	
	vector<IMGFatBlock*> fatBlocks;
	
}


class IMGFile {
public:
	IMGHeader header;

	unsigned char* data;
	
	vector<IMGFatBlock*> fatBlocks;
	
	
	void LoadPath(string* path);
};


IMGFatBlock::IMGFatBlock(unsigned char* data, size_t dlen) {
	start = data;
	
	name->assign(data + 0x1, 8);
	type->assign(data + 0x9, 3);
	
	fileSize = *((uint32_t*)(data + 0xc));
	
	subFileIndex = *((uint16_t*)(data + 0x10));
	
	memcpy(blockSequenceNumbers, data + 0x20, 480);
}



void IMGHeader::Parse(unsigned char* data, size_t len) {
	
	blockSize = 2 << (data[0x62] + data[0x61]);
	
	firstSubFileOffset = *((uint32_t*)&(data[0x40c]));
	fatSize = firstSubFileOffset - 0x600;
	fatTable = data + 0x600;
	
	memcpy(blockSequenceNumbers, data + 0x420, 480);
	
	
	/*
	0x49+20 file description, padded with 0x20
	0x65+31 file description, continued, padded with 0x20, terminated with \0
	
	0x61, 0x62 blok size exponents, bs=2^(61+62)  
	
	0x40C+4, first sub-file offset, absolute
	0x420+480, block sequence numbers
	*/
	
	while() {
		
		
	}
	
	
	
	
}



void IMGFile::LoadPath(string* path) {
	size_t flen;
	
	
	data = readFile(path->c_str(), &flen);
	if(!data) {
		cout << "could not open file: " << *path << endl;
		return;
	}
	
	if(flen < 446 + 66) { // header + partition table
		cout << "file is too small\n";
		return;
	}
	
	// check and decode the xor byte
	unsigned char xorbyte = data[0]; 
	printf("xor btye: %x\n", xorbyte);  
	
	if(xorbyte) {
		cout << "decoding file... ";
		
		for(size_t i = 0; i < flen; i++) {
			data[i] ^= xorbyte; 
		}
		
		cout << "done\n";
		
		string dp = *path + "decoded";
		writeFile(dp.c_str(), (void*)data, flen);
	}
	
	// parse the header
	header.Parse(data);
	
	
	
	
	
}











int main(int argc, char* argv[]) {
	IMGFile img;
	
	for(int i = 1; i < argc; i++) {
		if(argv[i][0] == '-') {
			if(argv[i][1] == '-') {
				// long form
				// -x --dexor
			}
			else { // short form
				
			}
		}
		else { // a file
			string s = argv[i];
			
			img.LoadPath(&s);
			
		}
		
		
		
	}

	
	return 0;
}
