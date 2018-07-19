#include "ansify.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{
	union ansify_colorkey key = {{-999, -999, -999}};
	int threshold = 0;

	int c;

	while ((c = getopt(argc, argv, "k:t:")) != -1) {
		switch (c) {
		case 'k'://colorkey
			if (sscanf(optarg, "%x:%x:%x", &key.r, &key.g, &key.b) < 3) {
				fprintf(stderr, "invalid input. -k %%x:%%x:%%x\n");
				key.r = key.g = key.b = -999;
			}
			break;
		case 't'://threshhold
			threshold = strtol(optarg, NULL, 10);
			if (errno) {
				fprintf(stderr, "invalid input. -t %%d\n");
				threshold = 10;
			}
			break;
		}
	}

	while (argv[optind]) {
		char *output = ansify(argv[optind], key, threshold);
		if (output == NULL) {
			if (errno) {
				perror("Error: ");
				errno = 0;
			} else {
				fputs("Unable to convert image\n", stderr);
			}
		} else {
			printf("%s\n", output);
		}
		optind++;
	}
	return EXIT_SUCCESS;
}
