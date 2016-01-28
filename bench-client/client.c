#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define BENCHMOD_NAME "/proc/benchmod"

int main()
{
	int fd, ret = 0;

	fd = open(BENCHMOD_NAME, O_RDONLY);

	if(fd == -1) {
		printf("Error opening %s\n", BENCHMOD_NAME);
	}

	if(ioctl(fd, 0) == -1) {
		ret = -1;
	}

	close(fd);

	return ret;
}
