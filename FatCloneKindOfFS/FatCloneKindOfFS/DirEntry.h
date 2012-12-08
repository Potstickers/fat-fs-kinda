#pragma once
#include <time.h>
class DirEntry
{
public:
	DirEntry(void);
	DirEntry(char*);
	~DirEntry(void);

	char *getFormattedEntry();

	char name[8];
	char ext[3];
	char attr;
	char month;
	char day;
	unsigned short year;
	char hour;
	char min;
	char sec;
	unsigned short filesize;
	unsigned short startBlock;

private:
	
};

