#ifndef _WIN32
#define _GNU_SOURCE 1
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"
#include "SDL_ttf.h"

#define WRITE_FILE	0	/* write to a file */
#define NGLYPHS		128	/* number of glyphs (all printable characters) */
/* Process the font.
 */
void process_font(TTF_Font *font)
{
	SDL_Surface *s = NULL;
	SDL_Color color = {255, 255, 255, 0};
	unsigned short ch;
	int temp;

	TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
	printf("#define BMP_NGLYPHS\t\t%d\t/* Number of glyphs */\n", NGLYPHS);
	printf("const unsigned char *BMP_bits[BMP_NGLYPHS] = {\n");
	for(ch = 0; ch < NGLYPHS; ch++) {
		temp = TTF_GlyphIsProvided(font, ch);
		if(temp) {
			s = TTF_RenderGlyph_Solid(font, ch, color);
			if(s) {
				unsigned char *pixels = s->pixels;
				int x, y;
				for(y = 0; y < s->h; y++) {
					printf("\t{ ");
					for(x = 0; x < s->pitch; x++) {
						printf("0x%x%s", *pixels++,
							(x == (s->pitch-1) ? "" : ", "));
					}
					printf(" }%s", (y < (s->h-1) ? ",\n" : "\n"));
				}
				SDL_FreeSurface(s);
			}
		}
	}
	printf("}\n};\n");
}
/* Program to make a C array out of a TTF font.
 */
int main(int argc, char **argv)
{
	TTF_Font *font;
	if(argc < 2 || argc > 3) {
		fprintf(stderr, "Usage: %s <font.ttf> <out-name>\n", argv[0]);
		return 1;
	}
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Error: %s\n", SDL_GetError());
		return 1;
	}
	atexit(SDL_Quit);
	if(TTF_Init() < 0) {
		fprintf(stderr, "Error: %s\n", TTF_GetError());
		return 1;
	}
	atexit(TTF_Quit);
	font = TTF_OpenFont(argv[1], 10);
	if(font == NULL) {
		fprintf(stderr, "Error: Could not open font.\n");
		return 1;
	}
	printf("Everything initialised!\n");
	process_font(font);
	TTF_CloseFont(font);
	return 0;
}
