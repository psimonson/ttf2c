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

/* some definitions */
static FT_Library library;
static FT_Face face;
static FT_Error err;
static FT_Int nglyphs;
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
static void to_bitmap(unsigned char **image, FT_Bitmap *bitmap,
	FT_Glyph_Metrics *metrics)
{
	FT_Int col_start = metrics->horiBearingX >> 6;
	FT_Int row_start = metrics->horiBearingY >> 6;
	FT_UInt y;
	FT_Int x;

	for(y = 0; y < metrics->height; y++) {
		FT_UInt row = row_start + y;
		for(x = 0; x < metrics->width; x++) {
			FT_Int col = col_start + x;
			if(col < 0 || col >= bitmap->pitch
				|| row >= bitmap->rows)
				continue;
			image[row][col] = bitmap->buffer[row*bitmap->pitch+col];
		}
	}
}
/* Make an output file with xbm extension.
 */
static void out_header(unsigned char **image, const char *name,
	unsigned int nglyphs, int pitch)
{
#if WRITE_FILE
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
	char filename[MAX_PATH];
	FILE *fp;
#endif
	unsigned int i;
	int x;

#if WRITE_FILE
	memset(filename, 0, sizeof(filename));
	snprintf(filename, sizeof(filename), "%s.h", name);
	if((fp = fopen(filename, "wb")) == NULL) {
		perror("out_xbm()");
		return;
	}
	fprintf(fp, "#define BMP_GLYPHS\t\t%d\t/* Number of glyphs */\n",
			nglyphs);
	fprintf(fp, "#define BMP_PITCH\t\t%d\t/* Number of glyphs */\n",
			pitch);
	fprintf(fp, "#define BMP_BPG\t\t\t%d\t/* Bytes per glyph */\n\n",
			pitch*3);
	fprintf(fp, "const unsigned char BMP_bits[%s][%s] = {\n",
			"BMP_GLYPHS", "BMP_BPG");
	for(i = 0; i < nglyphs; i++) {
		fprintf(fp, "\t{ ");
		for(x = 0; x < pitch*3; x+=3) {
			fprintf(fp, "0x%x, 0x%x, 0x%x%s",
				image[i][x],
				image[i][x+1],
				image[i][x+2],
				(x == (pitch*3-3) ? "" : ", "));
		}
		fprintf(fp, " }%s", (i == (nglyphs-1) ? "\n" : ",\n"));
	}
	fprintf(fp, "};\n");
	fclose(fp);
#else
#define UNUSED(x)
	UNUSED(name);
	printf("#define BMP_GLYPHS\t\t%d\t/* Number of glyphs */\n",
			nglyphs);
	printf("#define BMP_PITCH\t\t%d\t/* Number of glyphs */\n",
			pitch);
	printf("#define BMP_BPG\t\t\t%d\t/* Bytes per glyph */\n\n",
			pitch*3);
	printf("const unsigned char BMP_bits[%s][%s] = {\n",
			"BMP_GLYPHS", "BMP_BPG");
	for(i = 0; i < nglyphs; i++) {
		printf("\t{ ");
		for(x = 0; x < pitch*3; x+=3) {
			printf("0x%x, 0x%x, 0x%x%s",
				image[i][x],
				image[i][x+1],
				image[i][x+2],
				(x == (pitch*3-3) ? "" : ", "));
		}
		printf(" }%s", (i == (nglyphs-1) ? "\n" : ",\n"));
	}
	printf("};\n");
#undef UNUSED
#endif
}
/* Program to convert from ttf to C array.
 */
int main(int argc, char **argv)
{
	unsigned char **image;
	FT_GlyphSlot slot;
	int i, g, pitch;
	int glyph_index;

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
	if((err = FT_Set_Char_Size(face, 16*64, 16*64, 100, 100))) {
		fprintf(stderr, "Error: %s\n", FT_Error_String(err));
		FT_Done_Face(face);
		FT_Done_FreeType(library);
		exit(1);
	}
	glyph_index = FT_Get_Char_Index(face, 'A');
	if((err = FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER))) {
		fprintf(stderr, "Error: %s\n", FT_Error_String(err));
		FT_Done_Face(face);
		FT_Done_FreeType(library);
		exit(1);
	}
	nglyphs = face->num_glyphs;
	pitch = face->glyph->bitmap.pitch;
	image = malloc(sizeof(unsigned char *)*nglyphs);
	if(image == NULL) {
		FT_Done_Face(face);
		FT_Done_FreeType(library);
		exit(2);
	}
	for(i = 0; i < nglyphs; i++) {
		image[i] = malloc(sizeof(unsigned char)*pitch*3);
		if(image[i] != NULL)
			memset(image[i], 0, sizeof(unsigned char)*pitch*3);
	}
	for(g = 0; g < 256; g++) {
		glyph_index = FT_Get_Char_Index(face, g);
		err = FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER);
		if(err) {
			fprintf(stderr, "Warning: %s\n",
				get_error_string(err));
			continue;
		}
		slot = face->glyph;
		to_bitmap(image, &slot->bitmap, &slot->metrics);
	}
#if WRITE_FILE
	if(argc == 3)
		out_header(image, argv[2], nglyphs, pitch);
	else
		out_header(image, "font", nglyphs, pitch);
#else
	out_header(image, NULL, nglyphs, pitch);
#endif
	FT_Done_Face(face);
	FT_Done_FreeType(library);
	for(i = 0; i < nglyphs; i++)
		free(image[i]);
	free(image);
	return 0;
}
