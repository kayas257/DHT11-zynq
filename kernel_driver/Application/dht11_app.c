
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
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
/*
   This code reads the misc platform driver to read temperature from kernel
 */

#define ENABLE_DEVICE _IOR('a', 'a', int32_t *)
#define DISABLE_DEVICE _IOR('a', 'b', int32_t *)

int keepruning = 1;
struct dht_sensor_data
{
	uint32_t ctrl;
	uint32_t debug;
	uint32_t temp;
	uint32_t humidity;
	uint32_t crc;
};
void sig_handler(int signo)
{
	if (signo == SIGINT)
	{
		printf("received SIGINT\n");
		keepruning = 0;
	}
}
int main()
{
	struct dht_sensor_data buf;
	int number;
	if (signal(SIGINT, sig_handler) == SIG_ERR)
		;
	int fd = open("/dev/DH11", O_RDWR | O_SYNC); // Assuming that kernel driver is running
	float temp;
	if (fd == -1)
	{
		printf("ERROR \n");
	}
	ioctl(fd, ENABLE_DEVICE, (int *)&number);
	while (keepruning)
	{
		read(fd, &buf, sizeof(buf));
		temp = buf.temp + (buf.debug & (0x00ff0000 >> 16)) / 10;
		printf("kernel is: %f\n\r", temp);
		printf("debug data is: %x\n\r", buf.debug);
		printf("kernel humidity is: %u\n\r", buf.humidity);
		sleep(2);
	}
	ioctl(fd, DISABLE_DEVICE, (int *)&number);
}
