#ifndef _WIN32
#define _GNU_SOURCE 1
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"
#include "SDL_ttf.h"

#define WRITE_FILE	0	/* write to a file */
#define NGLYPHS		256	/* number of glyphs (all printable characters) */
/* Save glyph to array.
 */
void save_glyph(TTF_Font *font, unsigned short ch)
{
	SDL_Surface *s = NULL;
	SDL_Color color = {255, 255, 255, 255};
	int i, j;
	s = TTF_RenderGlyph_Solid(font, ch, color);
	if(s) {
		unsigned char *pixels = s->pixels;
		printf("\nconst unsigned char BMP_bits%d[%d] = {\n",
			ch, s->h*s->w);
		for(i = 0; i < s->h; i++) {
			printf("\t{ 0x%x", *pixels++);
			for(j = 1; j < s->pitch; j++) {
				printf(", 0x%x", *pixels++);
			}
			printf(" }%s\n", (i < s->h-1 ? "," : ""));
		}
		printf("};\n");
		SDL_FreeSurface(s);
	}
}
/* Build array from bits of glyphs.
 */
void build_array(TTF_Font *font)
{
	int i, temp;
	printf("\nconst unsigned char *BMP_bits[BMP_NGLYPHS] = { ");
	for(i = 0; i < NGLYPHS; i++) {
		temp = TTF_GlyphIsProvided(font, i);
		if(temp) {
			printf("{ BMP_bits%d }%s", i,
				(i < NGLYPHS-1 ? ", " : " "));
		} else {
			printf("{ NULL }%s", (i < NGLYPHS-1 ? ", " : " "));
		}
	}
	printf("};\n");
}
/* Process the font.
 */
void process_font(TTF_Font *font)
{
	unsigned short ch;
	int temp;

	TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
	printf("#define BMP_NGLYPHS\t\t%d\t/* Number of glyphs */\n", NGLYPHS);
	for(ch = 0; ch < NGLYPHS; ch++) {
		temp = TTF_GlyphIsProvided(font, ch);
		if(temp) {
			save_glyph(font, ch);
		}
	}
	build_array(font);
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
	process_font(font);
	TTF_CloseFont(font);
	return 0;
}
