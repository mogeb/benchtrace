#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <popt.h>
#include <time.h>

#define BENCHMOD_NAME "/proc/benchmod"

static int loop = 1;
static char *outfile = "default";
static char *tracer = "none";
static int clear = 0;

struct poptOption options[] = {
	{
		NULL,
		'n',
		POPT_ARG_INT | POPT_ARGFLAG_OPTIONAL,
		&loop,
		NULL,
		"Number of times to run the system call",
		"Desc"
	},
	{
		"output",
		'o',
		POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL,
		&outfile,
		NULL,
		"File to append to",
		"Desc file"
	},
	{
		"tracer",
		't',
		POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL,
		&tracer,
		NULL,
		"Tracer",
		"tracer"
	},
	{
		"clear",
		'c',
		POPT_ARG_NONE | POPT_ARGFLAG_OPTIONAL,
		&clear,
		NULL,
		"Clear file",
		"clear"
	},
	POPT_AUTOHELP
};

struct timespec ts_diff(struct timespec start, struct timespec end)
{
	struct timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

int main(int argc, char **argv)
{
	int i, fd, val, ret = 0;
	long latency;
	struct timespec beg, end, diff;
	FILE *file;
	poptContext pc;

	pc = poptGetContext(NULL, argc, (const char **)argv, options, 0);

	if (argc < 2) {
		poptPrintUsage(pc, stderr, 0);
		return 1;
	}

	while ((val = poptGetNextOpt(pc)) >= 0) {
		printf("poptGetNextOpt returned val %d\n", val);
	}

	fd = open(BENCHMOD_NAME, O_RDONLY);

	if(fd == -1) {
		printf("Error opening %s\n", BENCHMOD_NAME);
	}

	clock_gettime(CLOCK_MONOTONIC, &beg);
	for(i = 0; i < loop; i++) {
		ioctl(fd, 0);
	}
	clock_gettime(CLOCK_MONOTONIC, &end);

	close(fd);
	diff = ts_diff(beg, end);

	latency = diff.tv_sec * 1000000000 + diff.tv_nsec;
	latency = latency / loop;

	file = fopen(outfile, clear ? "w+" : "a+");
	if(clear) {
		fprintf(file, "tracer,total time,time per call\n");
	}
	fprintf(file, "%s,%ld,%ld\n", tracer, diff.tv_nsec, latency);
	fprintf(stdout, "%ld\n", diff.tv_nsec);
	close(file);

	return ret;
}
