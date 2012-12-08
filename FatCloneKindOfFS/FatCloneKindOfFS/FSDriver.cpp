#include "Disk.h"
#include <sstream>
using namespace std;

int main(){
	Disk *my_disk = Disk::Instance();
	//creation at root
	my_disk->mkDir("/var");
	my_disk->mkDir("/usr");
	my_disk->mkDir("/bin");
	my_disk->mkFile("/hello.c");
	my_disk->mkFile("/foo.bar");

	//creation in directories
	my_disk->mkDir("/var/log");
	my_disk->mkDir("/var/tmp");
	my_disk->mkDir("/var/food");
	my_disk->mkDir("/usr/stuff");
	//deletion
	my_disk->rmDir("/usr/stuff");
	my_disk->rmDir("/var/food");
	my_disk->rmFile("/foo.bar");
	//coealesce test
	//create 3 blocks worth
	for(int i = 1; i<=33; ++i){
		stringstream s;
		s << "/usr/stuff" << i;
		if(i%2==0){
			my_disk->mkDir(s.str());
		}else{
			s << ".txt";
			my_disk->mkFile(s.str());
		}
	}
	//delete
	for(int i = 17; i<=32; ++i){
		stringstream s;
		s << "/usr/stuff" << i;
		if(i%2==0){
			my_disk->rmDir(s.str());
		}else{
			s << ".txt";
			my_disk->rmFile(s.str());
		}
	}
	delete my_disk;
	return 0;
}