#pragma once
#include <fstream>
#include <iostream>
using namespace std;

class SuperBlock
{
public:
	SuperBlock(void);
	SuperBlock(fstream*);
	~SuperBlock(void);

	char *getFormattedBlock();//prepares a char[512] for writing back 
	//structure of superblock, typesizes here not representative of actual size 
	unsigned int magic_number;
	unsigned int block_size;
	unsigned int total_blocks;
	unsigned int fat_location;
	unsigned int root_location;
	unsigned int max_fat_address;
	unsigned int num_free_blocks;
};

