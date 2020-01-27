#ifndef _WIN32
#define _GNU_SOURCE 1
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"
#include "SDL_ttf.h"

#define WRITE_FILE	0	/* write to a file */

int main(int argc, char **argv)
{
	if(argc < 2 || argc > 3) {
		fprintf(stderr, "Usage: %s <font.ttf> <out-name>\n", argv[0]);
		return 1;
	}
	return 0;
}
