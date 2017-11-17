#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>

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


class IMGFile;

class IMGHeader {
public:
	
	size_t blockSize;
	
	size_t firstSubFileOffset;
	uint16_t blockSequenceNumbers[480/2];
	
	unsigned char* fatTable;
	size_t fatSize;
	
	size_t partitionTableEnd;
	
	uint8_t ptStartHead, ptStartSector, ptStartCylinder;
	uint8_t ptEndHead, ptEndSector, ptEndCylinder;
	uint32_t ptRelSectors, ptNumSectors;
	uint16_t hSectors, hHeads, hCylinders;
	uint16_t oneE;
	
	void Parse(unsigned char* data, size_t len);
	uint8_t* findPartitionTable(unsigned char* end_header, size_t flen);
};


class IMGSubFile;

class IMGFatBlock {
public:
	unsigned char* start;
	int blockIndex;
	uint16_t* blockIndices;
	
	
	string name;
	string type;
	string fullName;
	
	uint32_t fileSize;
	IMGSubFile* file;
	
	uint16_t subFileIndex;
	
	int numBlocks;
	IMGFatBlock* firstBlock;
	IMGFatBlock* nextBlock;
	
	uint16_t blockSequenceNumbers[480/2];
	
	
	IMGFatBlock(unsigned char* data, size_t dlen);
};



class IMGSubFile {
public:
	size_t size;
	
	string name;
	string type;
	string fullName;
	
	uint8_t* buffer;
	size_t fileSize;
	
	vector<IMGFatBlock*> fatBlocks;
	vector<uint16_t> blocks;
	
	IMGSubFile(IMGFatBlock* b);
	void AddFatBlock(IMGFatBlock* b);
	void Collect(IMGFile* f);
	void DumpTo(string dir); 
};


class IMGFile {
public:
	IMGHeader header;
	
	string fileName;
	string filePath;
	string fileDir;
	string dumpDir;

	unsigned char* data;
	
	vector<IMGFatBlock*> fatBlocks;
	
	map<string, IMGSubFile*> subFiles;
	
	void LoadPath(string* path);
	uint8_t* getBlock(uint16_t index);
	void insertFatBlock(IMGFatBlock* b);
	void DumpFiles();
};


IMGFatBlock::IMGFatBlock(unsigned char* data, size_t dlen) {
	start = data;
	
	name.assign((const char*)(data + 0x1), 8);
	type.assign((const char*)(data + 0x9), 3);
	fullName = name + "." + type;
	
	fileSize = *((uint32_t*)(data + 0xc));
	
	subFileIndex = *((uint16_t*)(data + 0x10));
	
	memcpy(blockSequenceNumbers, data + 0x20, 480);
	
	cout << "name: " << name << "." << type << endl;
	
}


// the partition table location is not consistent; look for the start manually
// a better implementation might only skip 0x0-filled chunks of space
uint8_t* IMGHeader::findPartitionTable(unsigned char* end_header, size_t flen) {
	
	// 1 followed by 11 0x20's
	unsigned char sig[] = {0x01, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 };
	
	return (uint8_t*)memmem(end_header, flen, sig, sizeof(sig));
}


void IMGHeader::Parse(unsigned char* data, size_t len) {
	
	blockSize = 1 << (data[0x62] + data[0x61]);
	
	
	hSectors = *((uint16_t*)&(data[0x18]));
	hHeads = *((uint16_t*)&(data[0x1a]));
	hCylinders = *((uint16_t*)&(data[0x1c]));
	oneE = *((uint16_t*)&(data[0x1e]));
	
	uint16_t fiveD = *((uint16_t*)&(data[0x5d]));
	uint16_t fiveF = *((uint16_t*)&(data[0x5f]));
	
	ptStartHead = data[0x1bf];
	ptStartSector = data[0x1c0];
	ptStartCylinder = data[0x1c1];
	
	ptEndHead = data[0x1c3];
	ptEndSector = data[0x1c4];
	ptEndCylinder = data[0x1c5];
	
	ptRelSectors = *((uint32_t*)&(data[0x1c6]));
	ptNumSectors = *((uint32_t*)&(data[0x1ca]));
	
	// this section is located at 0x400 in official gmapsupp files but 0x1000 in some others
	// nothing seems to indicate location, at light analysis, so we just search for the start
	uint8_t* ptend = findPartitionTable(data, len);
	
	firstSubFileOffset = *((uint32_t*)&(ptend[0x00c]));
	fatSize = firstSubFileOffset - (ptend - data); // wrong
	fatTable = ptend + 512;
	
	memcpy(blockSequenceNumbers, ptend + 0x020, 480);

	
	
	cout << "hSectors: " << (int)hSectors << endl; 
	cout << "hHeads: " << (int)hHeads << endl; 
	cout << "hCylinders: " << (int)hCylinders << "\n"; 
	cout << "fiveD: " << (int)fiveD << "\n"; 
	cout << "fiveF: " << (int)fiveF << "\n"; 
	cout << "oneE: " << (int)oneE << "\n\n"; 
	
	cout << "ptStartHead: " << (int)ptStartHead << endl; 
	cout << "ptStartSector: " << (int)ptStartSector << endl; 
	cout << "ptStartCylinder: " << (int)ptStartCylinder << "\n\n"; 
	
	cout << "ptEndHead: " << (int)ptEndHead << endl; 
	cout << "ptEndSector: " << (int)ptEndSector << endl; 
	cout << "ptEndCylinder: " << (int)ptEndCylinder << "\n\n"; 
	
	cout << "ptRelSectors: " << ptRelSectors << endl; 
	cout << "ptNumSectors: " << ptNumSectors << "\n\n"; 
	
	cout << "middle block location: " << (long)(ptend - data) << "\n\n"; 

	cout << "blockSize: " << blockSize << endl; 
	cout << "firstSubFileOffset: " << firstSubFileOffset << " = " << firstSubFileOffset / blockSize << " blocks" << endl; 
	
	
	
	/*
	0x49+20 file description, padded with 0x20
	0x65+31 file description, continued, padded with 0x20, terminated with \0
	
	0x61, 0x62 blok size exponents, bs=2^(61+62)  
	
	0x????C+4, first sub-file offset, absolute
	0x????20+480, block sequence numbers
	*/
	
	

	
	
}


IMGSubFile::IMGSubFile(IMGFatBlock* b) {
	AddFatBlock(b);
	if(b->subFileIndex != 0) {
		cout << "!!! initial subfile index is not 0 (" << b->fullName << ")\n";
	}
	fileSize = b->fileSize;
	fullName = b->fullName;
	type = b->type;
	name = b->name;
}

void IMGSubFile::AddFatBlock(IMGFatBlock* b) {
	fatBlocks.push_back(b);
	
	// TODO: build up block list
	for(int i = 0; i < 240; i++) {
		uint16_t bsn = b->blockSequenceNumbers[i];
		if(bsn == 0xffff) break;
		//cout << "   bsn: " << bsn << endl;
		blocks.push_back(bsn);
	}
	
}

// gathers all the blocks into one buffer
void IMGSubFile::Collect(IMGFile* f) {
	buffer = (uint8_t*)malloc(fileSize);
	uint8_t* p = buffer;
	
	size_t bytesCopied = 0;
	
	vector<uint16_t>::iterator it;
	for(it = blocks.begin(); it != blocks.end(); it++) {
		if(p - buffer >= fileSize) {
			cout << "!!! incorrect subfile size\n";
			break;
		}
		
		if(*it >= 0xffff) {
			cout << "!!! bad block index: " << *it << endl; 
		}
		
		uint8_t* d = f->getBlock(*it);
		
		size_t n = f->header.blockSize;
		if(fileSize - bytesCopied < f->header.blockSize) {
			n = fileSize - bytesCopied;
		}
		
		memcpy(p, d, n);
		
		p += f->header.blockSize;
		bytesCopied += f->header.blockSize;
	}
}




uint8_t* IMGFile::getBlock(uint16_t index) {
	return &data[index * header.blockSize];
}

void IMGFile::insertFatBlock(IMGFatBlock* b) {
	fatBlocks.push_back(b);
	
	try {
		IMGSubFile* sf = subFiles.at(b->fullName);
		sf->AddFatBlock(b);
	}
	catch(...) { // catch everything; exceptions are evil anyway
		subFiles[b->fullName] = new IMGSubFile(b);
	}
	
}

void IMGFile::LoadPath(string* path) {
	size_t flen;
	
	filePath = *path;
	
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
	header.Parse(data, flen);
	
	
	
	
	// load data on subfile locations
	unsigned char* p = header.fatTable;
	// fat entries are 512 bytes long
	for(;p < data + header.firstSubFileOffset; p += 512) {
		if(! *p) {
			// the first byte is a marker for dummy entries
			cout << "empty fat entry\n";
			continue;
		} 
		
		// BUG len should be less p's offset but isn't checked right now anyway
		IMGFatBlock* b = new IMGFatBlock(p, flen);
		insertFatBlock(b);
		
	}
	
	
	// gather all the sub file parts together 
	for(map<string, IMGSubFile*>::iterator it = subFiles.begin(); it != subFiles.end(); it++) {
		cout << "Collecting file " << it->first << "... ";
		it->second->Collect(this);
		cout << "done.\n";
	}
	
	
}


void IMGSubFile::DumpTo(string dir) {
	
	string filePath = dir + "/" + fullName;
	
	cout << "dumping file " << filePath << "... ";
	
	writeFile(filePath.c_str(), buffer, fileSize);
	
	cout << "done\n"; 
}



void IMGFile::DumpFiles() {
	
	for(map<string, IMGSubFile*>::iterator it = subFiles.begin(); it != subFiles.end(); it++) {
		it->second->DumpTo(dumpDir);
	}
	
}






int main(int argc, char* argv[]) {
	IMGFile img;
	char dump = 0;
	
	for(int i = 1; i < argc; i++) {
		if(argv[i][0] == '-') {
			if(argv[i][1] == '-') {
				// long form
				// -x --dexor
				

			}
			else { // short form
				if(argv[i] == string("-d")) {
					dump = 1;
				}
			}
		}
		else { // a file
			string s = argv[i];
			
			img.LoadPath(&s);
			
		}
	}

	if(dump) {
		char* rp = realpath(img.filePath.c_str(), NULL);
		char* rp2 = strdup(rp); // dirname may modify it
		img.fileDir.assign(dirname(rp)); 
		img.fileName.assign(basename(rp2)); 
		
		free(rp);
		free(rp2);
		cout << img.fileDir << endl;
		img.dumpDir = img.fileDir + "/" + img.fileName + "_files";
		
		// TODO error checking
		mkdir(img.dumpDir.c_str(), 0777);
		
		img.DumpFiles();
	}
	
	
	return 0;
}
