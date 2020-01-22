#ifndef _WIN32
#define _GNU_SOURCE 1
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H

#define WRITE_FILE	0	/* write to a file */
#define IMAGE_PITCH	27	/* how many bytes per character */

/* some definitions */
static FT_Library library;
static FT_Face face;
static FT_Error err;
static FT_Int nglyphs, pitch;
/* Get the error string from a freetype error.
 */
const char *get_error_string(FT_Error err)
{
	#undef __FTERRORS_H__
	#define FT_ERROR_START_LIST	switch(err) {
	#define FT_ERRORDEF(e,v,s)	case e: return s;
	#define FT_ERROR_END_LIST	}
	#include FT_ERRORS_H
	return "Unknown error.";
}
/* Put image to bitmap.
 */
static void to_bitmap(unsigned char *image[IMAGE_PITCH], FT_Bitmap *bitmap,
	FT_Int x, FT_Int y)
{
	FT_Int i, j, p, q;
	FT_Int x_max = x + bitmap->width;
	FT_Int y_max = y + bitmap->rows;

	for(i = y, q = 0; i < y_max; i++, q++) {
		for(j = x, p = 0; j < x_max; j++, p++) {
			if(i < 0 || j < 0 || i >= nglyphs || j >= pitch)
				continue;
			image[j][i] |= bitmap->buffer[q*bitmap->width+p];
		}
	}
}
/* Make an output file with xbm extension.
 */
static void out_xbm(unsigned char *image[IMAGE_PITCH], const char *name,
	FT_Int nglyphs,	FT_Int pitch)
{
#if WRITE_FILE
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
	char filename[MAX_PATH];
	FILE *fp;
#endif
	int x, y;

#if WRITE_FILE
	memset(filename, 0, sizeof(filename));
	snprintf(filename, sizeof(filename), "%s.h", name);
	if((fp = fopen(filename, "wb")) == NULL) {
		perror("out_xbm()");
		return;
	}
	fprintf(fp, "#define BMP_GLYPHS\t\t%d\n", nglyphs);
	fprintf(fp, "#define BMP_PITCH\t\t%d\n\n", pitch);
	fprintf(fp, "const unsigned char BMP_bits[%s][%s] = {\n",
			"BMP_GLYPHS", "BMP_PITCH");
	for(y = 0; y <= nglyphs; y++) {
		fprintf(fp, "\t{ ");
		for(x = 0; x <= pitch; x++) {
			fprintf(fp, "0x%x%s", image[y][x],
				(y == nglyphs && x == pitch ? "" : ", "));
		}
		fprintf(fp, "}%s", (y == nglyphs ? "" : ",\n"));
	}
	fprintf(fp, "\n};\n");
	fclose(fp);
#else
#define UNUSED(x)
	UNUSED(name);
	printf("#define BMP_GLYPHS\t\t%d\n", nglyphs);
	printf("#define BMP_PITCH\t\t%d\n\n", pitch);
	printf("const unsigned char BMP_bits[%s][%s] = {\n",
			"BMP_GLYPHS", "BMP_PITCH");
	for(y = 0; y <= nglyphs; y++) {
		printf("\t{ ");
		for(x = 0; x <= pitch; x++) {
			printf("0x%x%s", image[y][x],
				(y == nglyphs && x == pitch ? "" : ", "));
		}
		printf("}%s", (y == nglyphs ? "" : ",\n"));
	}
	printf("\n};\n");
#undef UNUSED
#endif
}
/* Program to convert from ttf to C array.
 */
int main(int argc, char **argv)
{
	unsigned char **image;
	FT_GlyphSlot slot;
	int pen_x, pen_y, g;
	int i, j;

	if(argc < 2 || argc > 3) {
		fprintf(stderr, "Usage: %s <font.ttf> <out-name>\n", argv[0]);
		return 1;
	}
	if((err = FT_Init_FreeType(&library))) {
		fprintf(stderr, "Error: Init_FreeType failed: %s\n",
			get_error_string(err));
		exit(1);
	}
	if((err = FT_New_Face(library, argv[1], 0, &face))) {
		fprintf(stderr, "Error: FT_New_Face failed: %s\n",
			get_error_string(err));
		FT_Done_FreeType(library);
		exit(1);
	}
	if((err = FT_Set_Char_Size(face, 50*64, 50*64, 100, 100))) {
		fprintf(stderr, "Error: %s\n", FT_Error_String(err));
		FT_Done_Face(face);
		FT_Done_FreeType(library);
		exit(1);
	}
	nglyphs = face->num_glyphs;
	image = (unsigned char **)malloc(nglyphs*sizeof(unsigned char *));
	if(image == NULL) {
		FT_Done_Face(face);
		FT_Done_FreeType(library);
		exit(2);
	}
	for(i = 0; i < nglyphs; i++) {
		image[i] = malloc(IMAGE_PITCH*sizeof(unsigned char));
		if(image[i] == NULL)
			break;
	}
	if(image[i] == NULL) {
		for(j = 0; j < i; j++)
			free(image[i]);
		FT_Done_Face(face);
		FT_Done_FreeType(library);
		exit(3);
	}
	pen_x = 100;
	pen_y = 100;
	memset(*image, 0, nglyphs*IMAGE_PITCH);
	for(g = 0; g < nglyphs; g++) {
		int glyph_index = FT_Get_Char_Index(face, g);
		err = FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER);
		if(err) {
			fprintf(stderr, "Warning: %s\n",
				get_error_string(err));
			continue;
		}
		slot = face->glyph;
		to_bitmap(image, &slot->bitmap, slot->bitmap_left,
			nglyphs-slot->bitmap_top);
		pen_x += slot->advance.x >> 6;
		pen_y += slot->advance.y >> 6;
	}
#if WRITE_FILE
	out_xbm(image, argv[2], face->num_glyphs, slot->bitmap.pitch);
#else
	out_xbm(image, NULL, face->num_glyphs, slot->bitmap.pitch);
#endif
	FT_Done_Face(face);
	FT_Done_FreeType(library);
	for(i = 0; i <= nglyphs; i++)
		free(image[i]);
	free(image);
	return 0;
}
