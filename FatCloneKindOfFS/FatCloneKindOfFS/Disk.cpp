#include "Disk.h"
Disk *Disk::instance=NULL;
Disk *Disk::Instance(){
	if(!instance) instance = new Disk("Drive2MB");
	return instance;
}
Disk::Disk(string diskname)
{
	m_disk.open(diskname, ios::binary | ios::out | ios::in );
	time_t rawtime = time(0);
	tm *timeinfo = localtime(&rawtime);
	if(!m_disk.is_open()){
		cout << "Unable to mount disk. Aborting.\n";
		system("pause");
		exit(0);
	}else{
		if(fsPresent()){
			cout << "Disk mounted. File system detected.";
		}else{
			formatDisk();
		}
		super_block = new SuperBlock(&m_disk);
		cout << getDirPath("/");
	}
	system("pause");
}

SuperBlock *Disk::getSuperBlock(){
	return super_block;
}

Disk::~Disk(void)
{
	delete super_block;
	m_disk.close();
}
//============================================================
//Protected
//============================================================
char *Disk::formatDirEntry(string path){
	char *ret = new char[32];
	unsigned int dot_idx = path.find('.');
	bool isDir = dot_idx==string::npos;
	memset(ret,0,32);
	time_t rawtime = time(0);
	tm *timeinfo = localtime(&rawtime);
	memcpy(ret+18,&(timeinfo->tm_sec),1);
	memcpy(ret+17,&(timeinfo->tm_min),1);
	memcpy(ret+16,&(timeinfo->tm_hour),1);
	memcpy(ret+14,&(timeinfo->tm_year+=1900),2);
	memcpy(ret+13,&(timeinfo->tm_mday),1);
	memcpy(ret+12,&(timeinfo->tm_mon),1);
	unsigned char temp = (isDir)?1:0; //file or directory?
	memcpy(ret+11,&(temp),1); //is directory or file
	unsigned short temp2 = 0;
	memcpy(ret+19,&(temp2),2);//filesize, init 0
	memcpy(ret+21,&(temp2=0xFFFF),2); //start address, init empty
	if(isDir) memcpy(ret,path.c_str(),path.length());
	else{
		string name = path.substr(0,dot_idx); //name
		memcpy(ret, name.c_str(), name.length());
		string ext = path.substr(dot_idx+1); //extension
		memcpy(ret+8, ext.c_str(), name.length());
	}
	return ret;
}
string Disk::getDirPath(string path){
	string ret;
	if(path.length() == 1) return ret;
	const size_t last_slash_pos = path.rfind('/');
	if (last_slash_pos != string::npos)
	{
		return ret = path.substr(0, last_slash_pos);
	}
	return ret;
}

string Disk::getNextDir(string path){
	size_t first_not_slash = path.find_first_not_of('/');
	if(string::npos != first_not_slash){
		size_t next_slash = path.find_first_of('/', first_not_slash);
		if(string::npos != next_slash){
			return path.substr(first_not_slash, next_slash);
		}
	}
	return "";
}

string Disk::getBaseName(string path){
	string ret;
	size_t i = path.rfind('/');
	if(string::npos != i && i != path.length()-1) 
		ret = path.substr(i+1);
	return ret;
}
queue<string> Disk::tokenizePath(string path){
	queue<string> tokens;
	size_t p0 = 0, p1 = string::npos;

	while(p0 != string::npos)
	{
		p1 = path.find_first_of('/', p0);
		if(p1 != p0)
		{
			string token = path.substr(p0, p1 - p0);
			tokens.push(token);
		}
		p0 = path.find_first_not_of('/', p1);
	}
	return tokens;
}
bool Disk::fsPresent(){
	char magic_number[4]; //first 4 bytes represent magic number
	m_disk.seekg(0, ios::beg);
	m_disk.read(magic_number,4);
	return  *(unsigned int*) magic_number == 345670;
	//return  *reinterpret_cast<unsigned int*>(magic_number) == 345670;
}

void Disk::formatDisk(){
	m_disk.seekg(0, ios::end);

	//calculate space and sizes
	unsigned int totalBlocks = m_disk.tellg()/512;
	unsigned int numBlocksLessSuper = totalBlocks - 1;
	unsigned int sizeOfFATInBytes = numBlocksLessSuper * 2; //2 byte addresses
	unsigned int sizeOfFATInBlocks = (sizeOfFATInBytes/512) + 1; //+1 for possible truncation
	unsigned int dataRegionSizeInBlocks = totalBlocks - (sizeOfFATInBlocks + 1);
	sizeOfFATInBytes = dataRegionSizeInBlocks * 2; //recalculate FAT as needed for data region
	unsigned newSizeOfFATInBlocks = sizeOfFATInBytes/512 + 1; 
	dataRegionSizeInBlocks += (sizeOfFATInBlocks - newSizeOfFATInBlocks); //expand data region for difference of fat sizes
	sizeOfFATInBlocks = newSizeOfFATInBlocks;
	unsigned int maxFatAddress = dataRegionSizeInBlocks - 1;

	//debug info
	cout << "============================\nDisk is formatting...\n============================\n" ;
	cout << "Size of disk used: " << totalBlocks * 512 <<"\n";
	cout << "Size of FAT in Bytes:" << sizeOfFATInBytes<<"\n";
	cout << "Size of FAT in Blocks:" << sizeOfFATInBlocks <<"\n";
	cout << "Size of data region in bytes:" << dataRegionSizeInBlocks * 512 <<"\n";
	cout << "Size of data region in blocks:" << dataRegionSizeInBlocks <<"\n";
	cout << "Max fat address:" << maxFatAddress << "\n";

	//write info to superblock
	m_disk.seekp(0, ios::beg);
	unsigned int fields = 345670;			//the magic number
	m_disk.write((char*)&fields,4); 
	fields = 512;							//the constant block size
	m_disk.write((char*)&fields,4); 
	fields = totalBlocks;					//total number of blocks in this disk
	m_disk.write((char*)&fields,4); 
	fields = 512;							//FAT location on disk, comes after the superblock
	m_disk.write((char*)&fields,4); 
	fields = 512 + (sizeOfFATInBlocks*512);	//root location on disk
	m_disk.write((char*)&fields,4); 
	fields = maxFatAddress;					//max fat address
	m_disk.write((char*)&fields,4);
	fields = dataRegionSizeInBlocks;		//current number of free blocks
	m_disk.write((char*)&fields,4); 

	//write root FAT address, first block is last block
	m_disk.seekp(512, ios::beg);
	unsigned short endOfCluster = 0xFFFF;
	m_disk.write((char*)&endOfCluster,2);

	cout << "============================\nDone Formatting.\n============================\n";
}

bool Disk::isNotPathToFile(string path){
	return path.find('.') == string::npos;
}
bool Disk::isValidDirName(string baseName){
	return (baseName.find('.') == string::npos && baseName.length() <= 8);
}
bool Disk::isValidFileName(string baseName){
	size_t dot_idx = baseName.find('.');
	if(dot_idx != string::npos){
		return (baseName.substr(0,dot_idx).length() <=8 && baseName.substr(dot_idx+1).length() <= 3);
	}return false;
}
bool Disk::isValidDirectoryPath(string path){
	return isNotPathToFile(path) && (path.find(' ') == string::npos);
}

bool Disk::checkDirPath(string path){
	if(!isValidDirectoryPath(path)) return false;
	string dirPath = getDirPath(path);
	string baseName = getBaseName(path);
	if(baseName.empty() || !isValidDirName(baseName)) {cout<<"Bad path."; return false;}
	return true;
}

bool Disk::checkFilePath(string path){
	if(path.empty() || isNotPathToFile(path)) return false;
	string dirPath = getDirPath(path);
	string baseName = getBaseName(path);
	if(baseName.empty() || !isValidFileName(baseName)) {cout<<"Bad path."; return false;}
	return true;
}
//============================================================================================================
//Utils
//============================================================================================================
/*Given a char directory entry [32], returns the fat address of the start block*/
unsigned int Disk::getDirEntryStart(char *entry_buf){
	return *(unsigned int*) (entry_buf+21);
}
/*Gets the on disk position of the block referenced by fatAdr*/
unsigned int Disk::getBlockPos(unsigned int fatAdr){
	return super_block->root_location + (512 * fatAdr);
}
/*Gets the on disk position of the fat cell given a fat address*/
unsigned int Disk::getFatPos(unsigned int fatAdr){
	return super_block->fat_location + (2 * fatAdr);
}
/*Given a block buffer [512], itereates through it to see if a free or duplicate slot for a dir entry exists.
If found returns the offset into the block, else 0xFFFFFFFFF*/
unsigned int Disk::findEmptyEntrySlotInBlock(char *block_buffer){
	unsigned int offsetIntoBlock = 0xFFFFFFFF;
	for(int i = 0; i < 512; i+=32){
		if(memcmp(block_buffer+i,"\0\0\0\0\0\0\0\0", 8) == 0){
			return i;
		}
	}
	return offsetIntoBlock;
}
/*Given a block buffer [512], iterates through it to see if a dir entry of name is found. 
returns the offset into the block by 32 if found. else 0xFFFFFFFF*/
unsigned int Disk::findDirEntryInBlock(char *block_buffer, string name){
	unsigned int offsetIntoBlock = 0xFFFFFFFF;
	if(isValidFileName(name)){
		int dot_idx = name.find('.');
		string ext = name.substr(dot_idx+1);
		string filename = name.substr(0,dot_idx);
		for(int i = 0; i < 512; i+=32){
			if(memcmp(block_buffer+i,name.c_str(), strnlen(filename.c_str(),8)) == 0 && 
				memcmp(block_buffer+i+8, ext.c_str(), strnlen(ext.c_str(),3))==0){
				return i;
			}
		}
	}else{
		for(int i = 0; i < 512; i+=32){
			if(memcmp(block_buffer+i,name.c_str(), strnlen(name.c_str(),8)) == 0){
				return i;
			}
		}
	}
	return offsetIntoBlock;
}
/*Given a position of a block on disk, translates it to a fat address. 
pos should be a a multiple of 512 after subtracting the root location.*/
unsigned int Disk::getFatAdrFromBlockPos(unsigned int pos){
	return (pos-super_block->root_location) / 512;
}
/*Given a direntry position on disk, rounds it down to a its containing block pos*/
unsigned int Disk::roundToPrevBlockPos(unsigned int ret_pos){
	if(ret_pos - super_block->root_location < 512) return super_block->root_location;
	else{
		while((ret_pos - super_block->root_location) % 512 != 0) 
			ret_pos -=32;
	}
	return ret_pos;
}

unsigned int Disk::seekToDirEntry(string path){

	queue<string> tokenizedPath = tokenizePath(path);
	unsigned short curFATAddress = 0;
	if(tokenizedPath.empty()){
		return 0;
	}
	m_disk.seekg(super_block->root_location, ios::beg).tellg();
	char block_buffer[512];
	unsigned int offset;
	string nextTokenInPath;
	while(true){
		m_disk.read(block_buffer, 512).tellg();
		nextTokenInPath = tokenizedPath.front();
		offset = findDirEntryInBlock(block_buffer, nextTokenInPath.c_str());
		if(offset != 0xFFFFFFFF){
			tokenizedPath.pop();
			if(tokenizedPath.empty()){
				return m_disk.seekg(-512+offset, ios::cur).tellg();
			}else{
				curFATAddress = getDirEntryStart(block_buffer+offset);
				m_disk.seekg(getBlockPos(curFATAddress), ios::beg);
				continue;
			}
		}else{
			m_disk.seekg(getFatPos(curFATAddress), ios::beg);
			m_disk.read((char*)&curFATAddress, 2);
			if(curFATAddress == 0xFFFF){
				return 0xFFFFFFFF;
			}
			m_disk.seekg(getBlockPos(curFATAddress), ios::beg);
		}
	}
}
/*Given a block buffer, checks to see if it contains nothing but 0s returns true if so.
Works with direntries in mind. With files, its less likely contents will be sparse so this should work acceptably.*/
bool Disk::blockIsEmpty(char* block_buffer){
	for(int i = 0; i<512; i+=32){
		if(memcmp(block_buffer+i, "\0\0\0\0\0\0\0\0", 8)!=0) return false;
	}return true;
}
/*Returns a FAT address to a free block if any, else 0xFFFFFFFF*/
unsigned int Disk::getFreeBlock(){
	if(super_block->num_free_blocks == 0){
		return 0xFFFFFFFF;
	}
	unsigned short i = 1;
	unsigned short cur_address;
	unsigned short end_marker = 0xFFFF;
	m_disk.seekg(super_block->fat_location + 2, ios::beg);
	while(true){
		m_disk.read((char*)&cur_address,2);
		if( cur_address == 0x0000){
			m_disk.seekp(-2, ios::cur).write((char*)&end_marker,2);
			super_block->num_free_blocks--;
			return i;
		}
		if(i > super_block->max_fat_address){
			return 0xFFFFFFFF;
		}
		i++;
	}
}
//===================================================
//interface helpers
//===================================================
/*Given a base name, writes to disk a new dir entry at new entry pos. */
void Disk::getBlockChain(vector<unsigned short> &vec, unsigned short cur_adr){
	while(cur_adr != 0xFFFF){
		vec.push_back(cur_adr);
		m_disk.seekg(getFatPos(cur_adr), ios::beg).read((char*)&cur_adr, 2);
	}
}
void Disk::writeDirEntry(string baseName, unsigned int new_entry_pos){
	char *direntry = formatDirEntry(baseName);
	m_disk.seekp(new_entry_pos, ios::beg).write(direntry, 32);
	delete direntry;
}

bool Disk::mkDirEntryAtRoot(string baseName){
	unsigned short curFatPos = getFatPos(0);
	unsigned short curFatAdr = 0;
	unsigned short nextFatAdr = 0;
	unsigned int new_entry_pos = 0xFFFFFFFF;
	char block_buffer[512];
	do{
		curFatAdr = nextFatAdr;
		m_disk.seekg(getBlockPos(curFatAdr)).read(block_buffer, 512);
		if(findDirEntryInBlock(block_buffer, baseName.c_str()) == 0xFFFFFFFF){
			unsigned int offsetInBlock = findEmptyEntrySlotInBlock(block_buffer);
			if(offsetInBlock!=0xFFFFFFFF){
				new_entry_pos = m_disk.seekg(getBlockPos(curFatAdr)+offsetInBlock, ios::beg).tellg();
				break;
			}else{
				m_disk.seekg(getFatPos(curFatAdr), ios::beg).read((char*)&nextFatAdr,2);
			}
		}else{
			return false;
		}
	}while(nextFatAdr != 0xFFFF);
	//couldnt find a free slot in existing blocks
	if(new_entry_pos == 0xFFFFFFFF){
		unsigned short free_address = getFreeBlock();
		if(free_address == 0xFFFF){
			return false;
		}else{//got a free block
			//write the new free address to the prev fat address to point to the new one
			m_disk.seekp(getFatPos(curFatAdr), ios::beg).write((char*)&free_address, 2);
			new_entry_pos = getBlockPos(free_address);
		}
	}
	writeDirEntry(baseName, new_entry_pos);
	return true;
}

bool Disk::rmDirEntryAtRoot(string path){
	unsigned int direntry_pos = seekToDirEntry(path);
	if(direntry_pos!=0xFFFFFFFF){
		unsigned short start_address = 0;
		m_disk.seekg(direntry_pos+21,ios::beg).read((char*)&start_address,2); //get the start address
		if(start_address != 0xFFFF) return false; //directory is not empty
		else{
			char entry_buffer[32];
			memset(entry_buffer, 0 ,32);
			m_disk.seekp(-23, ios::cur).write(entry_buffer,32);
			coalesceBlocks(m_disk.seekp(-32,ios::cur).tellp(), getFatPos(getFatAdrFromBlockPos(roundToPrevBlockPos(direntry_pos))));
			return true;
		}
	}return false;
}
/*Given cur_pos, a position of a dir entry in the data region and an optional 
res_pos, the position of start refence in a dir entry if the directory will be empty. 
Shifts the blocks in the block chain starting at block located by cur_pos so that,
there won't be a gapping hole in the middle of a block chain. Usage: usually after a rm operation.*/
void Disk::coalesceBlocks(unsigned int cur_pos, unsigned int ref_pos){
	unsigned int cur_block_pos = roundToPrevBlockPos(cur_pos);
	char block_buffer[512];
	m_disk.seekg(cur_block_pos, ios::beg).read(block_buffer,512);
	if(blockIsEmpty(block_buffer)){
		unsigned short cur_FAT_Adr = getFatAdrFromBlockPos(cur_block_pos);
		unsigned short next_FAT_Adr; 
		m_disk.seekg(getFatPos(cur_FAT_Adr), ios::beg).read((char*)&next_FAT_Adr,2);
		if(next_FAT_Adr == 0xFFFF){
			m_disk.seekp(-2, ios::cur).write((char*)&(next_FAT_Adr=0x0000),2); //mark block as free
			m_disk.seekp(ref_pos, ios::beg).write((char*)&(next_FAT_Adr = 0xFFFF),2); //the direntry with its start reference here is now empty.
		}else{
			//coalesce chains
			unsigned short save_cur;
			while(next_FAT_Adr!=0xFFFF){
				m_disk.seekg(getBlockPos(next_FAT_Adr)).read(block_buffer, 512); //read the next block
				m_disk.seekp(getBlockPos(cur_FAT_Adr)).write(block_buffer,512); //write to current block
				save_cur = cur_FAT_Adr;
				cur_FAT_Adr = next_FAT_Adr; //advance the link
				m_disk.seekg(getFatPos(next_FAT_Adr)).read((char*)&next_FAT_Adr, 2).tellg(); //read the next reference address
			}
			m_disk.seekp(getFatPos(save_cur), ios::beg).write((char*)&next_FAT_Adr,2);
			makeFreeBlock(cur_FAT_Adr);
		}
	}
}
/*Given a FAT address that contains the final marker, marks it as free and clears the block at the address.*/
void Disk::makeFreeBlock(unsigned short fat_address){
	m_disk.seekp(getFatPos(fat_address));
	unsigned short free_marker = 0x0000;
	m_disk.write((char*)&free_marker,2);
	char empty_block[512];
	memset(empty_block,0,512);
	m_disk.seekp(getBlockPos(fat_address)).write(empty_block,512);
}
bool Disk::rmDirEntry(string path){
	string dirPath = getDirPath(path);
	string baseName = getBaseName(path);
	if(dirPath.empty() || dirPath == "/"){
		return rmDirEntryAtRoot(path);
	}
	//save possibly needed info from parent directory
	char entry_buffer[32];
	unsigned int saveBegAdrPosInParent = seekToDirEntry(dirPath); 
	if(saveBegAdrPosInParent == 0xFFFFFFFF) return false; //dir dne
	m_disk.seekg(saveBegAdrPosInParent).read(entry_buffer, 32); //read the direntry into buffer
	saveBegAdrPosInParent = (unsigned int) m_disk.tellg()-32+21; //the position where the stored start fat address in parent
	unsigned short saveBegAdrInParent;
	memcpy(&saveBegAdrInParent, entry_buffer+21, 2); //the actual start fat address in parent

	//goto actual dir to be deleted's entry
	m_disk.seekg(seekToDirEntry(path)).read(entry_buffer, 32);
	if(*(unsigned short*)(entry_buffer+21) != 0xFFFF){ //directory still has contents
		return false;
	}else{
		memset(entry_buffer, 0, 32);
		m_disk.seekp(-32, ios::cur).write(entry_buffer,32);
	}//directory entry is now gone

	//now to coalesce space
	coalesceBlocks(m_disk.seekp(-32, ios::cur).tellp(),saveBegAdrPosInParent);
	return true;
}
bool Disk::mkDirEntry(string path){
	string dirPath = getDirPath(path);
	string baseName = getBaseName(path);
	if(dirPath.empty() || dirPath == "/") return mkDirEntryAtRoot(baseName);

	unsigned int parent_entry_pos = seekToDirEntry(dirPath);
	char parent_entry_buf[32];
	m_disk.seekg(parent_entry_pos, ios::beg).read(parent_entry_buf, 32);
	unsigned int parent_entry_start = getDirEntryStart(parent_entry_buf);
	//======At the parent directory's entry at this point==================
	unsigned int new_entry_pos = 0xFFFFFFFF;
	unsigned int save_prev_FAT_pos;
	if(parent_entry_start != 0xFFFF){
		unsigned short prevFatPos = getFatPos(parent_entry_start);
		unsigned short prevFatAdr = parent_entry_start;
		char block_buffer[512];
		do{
			m_disk.seekg(getBlockPos(prevFatAdr), ios::beg).read(block_buffer,512);
			if(findDirEntryInBlock(block_buffer, baseName.c_str())==0xFFFFFFFF){
				new_entry_pos = findEmptyEntrySlotInBlock(block_buffer);
				if(new_entry_pos != 0xFFFFFFFF){
					new_entry_pos = (unsigned int) m_disk.tellg() - 512 + new_entry_pos;
				}
			}else return false;
			save_prev_FAT_pos = prevFatPos;
			m_disk.seekg(prevFatPos, ios::beg).read((char*)&prevFatAdr,2);
			prevFatPos = getFatPos(prevFatAdr);
		}while(prevFatAdr != 0xFFFF);
	}else{
		//=====Parent directory was not allocated free blocks============
		unsigned short free_address = getFreeBlock();
		if(free_address == 0xFFFF){
			return false;
		}else{//got a free block
			//write the new free address to the parent's entry
			memcpy(parent_entry_buf+21,&free_address, 2);
			m_disk.seekp(parent_entry_pos, ios::beg).write(parent_entry_buf, 32);
			new_entry_pos = getBlockPos(free_address);
		}
	}
	//=======Require a free block=================
	if(new_entry_pos == 0xFFFFFFFF){
		unsigned short free_address = getFreeBlock();
		if(free_address == 0xFFFF){
			return false;
		}else{//got a free block
			//write the new free address to the prev fat address to point to the new one
			m_disk.seekp(save_prev_FAT_pos, ios::beg).write((char*)&free_address, 2);
			new_entry_pos = getBlockPos(free_address);
		}
	}
	//======Finally write========================
	writeDirEntry(baseName, new_entry_pos);
	return true;
}

unsigned int Disk::seekToBlock(unsigned int fat_address){
	return m_disk.seekg(getBlockPos(fat_address), ios::beg).tellg();
}

char *Disk::readDirEntry(string path){
	unsigned int pos = seekToDirEntry(path);
	if(pos==0xFFFFFFFF) return NULL;
	else{
		char *ret = new char[32];
		m_disk.seekg(pos, ios::beg).read(ret,32);
		return ret;
	}
}

void Disk::writeBackDirEntry(char *direntry_buffer, unsigned int pos){
	m_disk.seekp(pos, ios::beg).write(direntry_buffer, 32);
}

unsigned int Disk::read(char *buffer, unsigned int num_bytes, unsigned int pos){
	m_disk.seekg(pos,ios::beg).read(buffer, num_bytes);
	return m_disk.gcount();
}

unsigned int Disk::write(char *buffer, unsigned int num_bytes, unsigned int pos){
	m_disk.seekp(pos,ios::beg).write(buffer, num_bytes);
	return m_disk.gcount();
}
//===================================================
//Interface functions
//===================================================
void Disk::ls(string path){
	//todo
}
bool Disk::mkDir(string path){
	if(checkDirPath(path) == false) return false;
	return mkDirEntry(path);
}

bool Disk::rmDir(string path){
	if (checkDirPath(path) == false) return false;
	return rmDirEntry(path);
}

bool Disk::mkFile(string path){
	if(checkFilePath(path) == false) return false;
	return mkDirEntry(path);
}

bool Disk::rmFile(string path){
	if (checkFilePath(path) == false) return false;
	return rmDirEntry(path);
}

