#include "FileHandle.h"


FileHandle::FileHandle(void)
{
	direntry = NULL;
	inst = Disk::Instance();
}


FileHandle::~FileHandle(void)
{
	delete direntry;
}

bool FileHandle::open(string path){
	direntry_pos = inst->seekToDirEntry(path);
	if(direntry_pos == 0xFFFFFFFF) return false;
	else{
		direntry = inst->readDirEntry(path); //read its directory entry
		inst->getBlockChain(block_chain,*(unsigned short*)direntry+21); //get block chain
		read_iterator = block_chain.begin();
		write_iterator = block_chain.begin();
		cur_read_adr = *read_iterator;
		cur_write_adr = *write_iterator;
		r_bytes_remaining = 512;
		w_bytes_remaining = 512;
		//todo: make sure everything needed is declared+initialized
	}
	return true;
}


int FileHandle::write(char *buffer, unsigned int bytes_to_write){
	//todo:
	//ensure differences between block limits and bytes to write are preserved
	//using w_bytes_remaining fields, track numbytes left to write for the current block
	//move iterators as needed during this process,
	//if writing beyond current file size, maybe adapt current getFreeBlock(), no blocks free? throw
	//append to block chain to write back into fat
	//imeplement modules to calculate nextWritePos() with available info
	//record bytes written 
	return 9001;
}

int FileHandle::read(char *buffer, unsigned int bytes_to_read){
	//todo: similarto write except less things to track
	//mainly just check: r_bytes_remaining for current block, ensure differences.
	//move iterators as needed during this process, switching blocks reset 
	//if read iterator is at end, return eof
	return 9001;
}

void FileHandle::close(){
	//todo:
	//if bytes written written this session > 0
	//write back block chain
	//format dir entry and write back
	//if bytes written this session < file size (value read in from directory entry on open)
	//see if coalescing needed. (implement calculations)
}