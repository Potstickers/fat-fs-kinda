#include "SuperBlock.h"


SuperBlock::SuperBlock(fstream *disk)
{
	char field_buffer[4];
	disk->seekg(0, ios::beg);
	disk->read(field_buffer, 4);
	magic_number = *(unsigned int*) field_buffer;
	disk->read(field_buffer, 4);
	block_size = *(unsigned int*) field_buffer;
	disk->read(field_buffer, 4);
	total_blocks = *(unsigned int*) field_buffer;
	disk->read(field_buffer, 4);
	fat_location = *(unsigned int*) field_buffer;
	disk->read(field_buffer, 4);
	root_location = *(unsigned int*) field_buffer;
	disk->read(field_buffer, 4);
	max_fat_address = *(unsigned int*) field_buffer;
	disk->read(field_buffer, 4);
	num_free_blocks = *(unsigned int*) field_buffer;
}

char *SuperBlock::getFormattedBlock(){
	char *formattedBlock = new char[512];
	memset(formattedBlock,0,512);
	memcpy(formattedBlock, (char *)magic_number,4);
	memcpy(&formattedBlock[4], (char *)block_size,4);
	memcpy(&formattedBlock[8], (char *)total_blocks,4);
	memcpy(&formattedBlock[12], (char *)fat_location,4);
	memcpy(&formattedBlock[16], (char *)root_location,4);
	memcpy(&formattedBlock[20], (char *)max_fat_address,4);
	memcpy(&formattedBlock[24], (char *)num_free_blocks,4);
	return formattedBlock;
}
SuperBlock::~SuperBlock(void)
{
}
