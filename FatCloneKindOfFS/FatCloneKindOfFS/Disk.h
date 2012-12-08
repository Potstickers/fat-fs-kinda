#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <queue>
#include <vector>
#include "SuperBlock.h"

/**
	The implementation only supports absolute paths.
	Some terminology, "address/adr" = FAT index, "position/pos" = actual position on file(disk).
	0xFFFFFFFF is used most of the time to indicate something is not found for function returning unsigned types.
*/
using namespace std;
class Disk
{
public:
	static Disk* Instance();
	~Disk(void);
	SuperBlock *getSuperBlock();

	//io interface
	bool mkDir(string);
	void ls(string); //not yet implemented
	bool rmDir(string);
	bool mkFile(string);
	bool rmFile(string);
	
	//helpers for io
	void getBlockChain(vector<unsigned short>&,unsigned short); //given avector to contain chain addresses and  starting FAT address, returns its entire block chain
	unsigned int seekToBlock(unsigned int); //given a fat address, seeks to the position for its corresponding block and returns the position
	unsigned int seekToDirEntry(string); //given a path to a directory entry, returns the file position of that entry if found, else 0xFFFFFFFF
	void writeDirEntry(string,unsigned int); //given a name of a file or directory and a position of a free directory entry, creates a new entry for it at position
	bool mkDirEntryAtRoot(string);	//given a path, returns true if successfully created a new entry at root e.g paths such as "/xyz"
	bool rmDirEntryAtRoot(string);	//given a path, returns true if successfully deleted an existing entry at root
	bool mkDirEntry(string); //given a path, returns true if successfully created a new entry
	bool rmDirEntry(string); //given a path, returns true if successfully deleted an existing entry
	char *readDirEntry(string); //given a path. returns a char[32] of the directory entry indentified by path.
	void writeBackDirEntry(char*,unsigned int); //given a char[32] and position on disk where entry is located, writes back char[32] dir entry.
	unsigned int read(char*,unsigned int,unsigned int); //given a buffer and number of bytes to read and position to read from, reads num bytes into buffer at position and returns number of bytes read.
	unsigned int write(char*, unsigned int, unsigned int); //given a buffer and number of bytes to write and position to write to, writes num bytes from buffer at position and returns number of bytes written.
	void coalesceBlocks(unsigned int, unsigned int); //given a pos of a directory entry and a position of a place that references the directory entry, coalesces its block chain.
	unsigned int getBlockPos(unsigned int);	//given a FAT address(index into FAT), returns the position in file(disk) where to start reading the block referenced by this address. 
	unsigned int getFatPos(unsigned int);	//given a FAT address(index into FAT), returns the position in file(disk) where to start reading the next address stored there.
protected:
	///////////////////////////////////////////////////
	/*string parsing/validating and formatted output*/
	//////////////////////////////////////////////////
	char *formatDirEntry(string);
	string getDirPath(string);
	string getBaseName(string);
	string getNextDir(string);
	queue<string> tokenizePath(string);
	bool isNotPathToFile(string);
	bool isValidDirectoryPath(string);
	bool isValidDirName(string);
	bool isValidFileName(string);
	bool checkDirPath(string);
	bool checkFilePath(string);
	////////
	/*meta*/
	////////
	bool fsPresent(); //checks if a given file(disk) has been formatted
	void formatDisk(); //formats a blank file(disk)
	/////////////////////////////////////////////////////////////////
	/*utils and translation between fat address <-> block positions*/
	/////////////////////////////////////////////////////////////////
	bool blockIsEmpty(char*); //Given a char[512], returns true if every char is 0, else false;
	unsigned int getDirEntryStart(char*); //Given a char[32], returns the starting FAT address found in the entry
	unsigned int findEmptyEntrySlotInBlock(char*); //Given a char[512], returns the offset into the block that is empty, 0xFFFFFFFF otherwise
	unsigned int findDirEntryInBlock(char*, string); //Given a char[512], and string directory entry name, returns the offset into block where entry is found, 0xFFFFFFFFF otherwise
	unsigned int roundToPrevBlockPos(unsigned int); //given a position in file(disk), returns the position of its containing block.
	unsigned int getFatAdrFromBlockPos(unsigned int); //given a position in file(disk), returns the fat address pointing to the block at given position.
	unsigned int getFreeBlock(); //searches for a free block and returns the fat address if found, else 0xFFFFFFFF
	void makeFreeBlock(unsigned short);	//given a FAT address that has been emptied out, returns it to the free pool
	
private:
	fstream m_disk; //the "disk"
	SuperBlock *super_block;
	Disk(string);
	Disk();
	Disk(const Disk&);
	Disk& operator= (const Disk&);
	static Disk* instance; //singleton
};

