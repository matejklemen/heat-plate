#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
	int width, height;
	if(argc == 3) {
		width = strtol(argv[1], (char**)NULL, 10);
		height = strtol(argv[2], (char**)NULL, 10);
	}
	else
		printf("Usage: \"%s <width> <height>\"\n", argv[0]);

	return 0;
}