#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>

#define MAP_SIZE 4096UL
#define BASEADDRESS
#define MAP_MASK (MAP_SIZE - 1)

void * getvaddr(int phy_addr)
{
	void *mmap_base;
	int memfd;
	void *mapped_dev_base;
	off_t dev_base= phy_addr;
	memfd=open("/dev/mem",O_RDWR| O_SYNC);
	if (memfd==-1)
	{
		printf("ERROR \n");
		exit(0);
	}

	mmap_base=mmap(0,MAP_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,memfd,dev_base & ~MAP_MASK);
	if(mmap_base==(void *) -1)
	{printf("ERROR \n");

	 exit(0); }
	mapped_dev_base=mmap_base+(dev_base&MAP_MASK);
	return mapped_dev_base;
}

int main()
{
	int * output=getvaddr(0x43C10000);
	while(1)
	{
		printf("valus is %x\n\r",*(output+1));
		printf("Temperature is %u\n\r",*(output+2));
		printf("Humidity  is %u\n\r",*(output+3));
		sleep(2);
	}
}

