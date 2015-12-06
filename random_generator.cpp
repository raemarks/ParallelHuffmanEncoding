#include <string>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>


#define MB 1000000

using std::string;

void
fill_buffer(int size, char *buf)
{
	srand(time(NULL));

	for (int i = 0; i < size; i++) {
		buf[i] = (char) (rand() % 10);
	}
}

/* TODO: have a frequency spread from a file. */
int
main(int argc, char* argv[])
{
	if (argc < 3) {
		printf("Usage: fgen [filename] [size in MB]\n");
		exit(1);
	}

	string filename = string(argv[1]);
	int sizeMB = std::stoi(argv[2]);

	char *buf = (char *) malloc(sizeof(char)*MB);

	int fd = open(filename.c_str(), O_WRONLY|O_CREAT, 0666);
	uint64_t n = 0;

	for (n = 0; n < ((uint64_t)sizeMB)*MB; /**/) {
		fill_buffer(MB, buf);
		n += write(fd, buf, MB);
	}

	printf("Wrote %dMB to %s\n", sizeMB, filename.c_str());

	return 0;
}
