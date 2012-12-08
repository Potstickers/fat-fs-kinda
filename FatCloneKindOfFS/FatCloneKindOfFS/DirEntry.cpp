#include "DirEntry.h"

DirEntry::DirEntry(char *name){
	time_t rawtime = time(0);
	tm *timeinfo = localtime(&rawtime);


}

DirEntry::DirEntry(void)
{
}


DirEntry::~DirEntry(void)
{
}
