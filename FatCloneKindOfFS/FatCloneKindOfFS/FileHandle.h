#pragma once
#include <string>
#include <iostream>
#include "Disk.h"
class FileHandle
{
public:
	FileHandle(void);
	~FileHandle(void);

	//File ops
	bool open(string); 
	void close();
	int read(char*, unsigned int);
	int write(char*, unsigned int);

private:
	vector<unsigned short> block_chain; //the block chain
	vector<unsigned short>::iterator read_iterator; //track which block currently reading from
	vector<unsigned short>::iterator write_iterator; //track which block currently writing to
	unsigned int r_bytes_remaining; //the number of bytes remaining in currenttly reading from block;
	unsigned int w_bytes_remaining; //the number of bytes remaining in currently writing to block;
	unsigned int write_pos; //the ondisk position the next write will be
	unsigned int read_pos;	// the ondisk position the next read will be
	unsigned short cur_read_adr; //the block which file is currently reading from
	unsigned short cur_write_adr; //the block whcih file is currently writing to
	Disk *inst; //get instance of disk
	char *direntry; //in memory directory entry of opened file
	unsigned int direntry_pos; //the on disk position where to write back the directory entry for file.
	unsigned short bytes_written; //local to the time a file is opened
};

