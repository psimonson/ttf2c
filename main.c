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

#define WRITE_FILE	1
#define WIDTH		640
#define HEIGHT		480

/* some definitions */
static unsigned char image[HEIGHT][WIDTH];
static FT_Library library;
static FT_Face face;
static FT_Error err;
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
static void to_bitmap(FT_Bitmap *bitmap, FT_Int x, FT_Int y)
{
	FT_Int i, j, p, q;
	FT_Int x_max = x + bitmap->width;
	FT_Int y_max = y + bitmap->rows;

	for(i = x, p = 0; i < x_max; i++, p++) {
		for(j = y, q = 0; j < y_max; j++, q++) {
			if(i < 0 || j < 0 || i >= WIDTH || j >= HEIGHT)
				continue;
			image[j][i] |= bitmap->buffer[q*bitmap->width+p];
		}
	}
}
/* Make an output file with xbm extension.
 */
static void out_xbm(const char *name, int w, int h)
{
#if WRITE_FILE
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
	char filename[MAX_PATH];
#endif
	FILE *fp;
	int x, y;

#if WRITE_FILE
	memset(filename, 0, sizeof(filename));
	snprintf(filename, sizeof(filename), "%s.h", name);
	if((fp = fopen(filename, "wb")) == NULL) {
		perror("out_xbm()");
		return;
	}
	fprintf(fp, "#define BMP_WIDTH\t%d\n", WIDTH);
	fprintf(fp, "#define BMP_HEIGHT\t%d\n\n", HEIGHT);
	fprintf(fp, "static char BMP_bits[] = {\n");
	for(y = 0; y < h; y++) {
		printf("\t");
		for(x = 0; x < w; x++) {
			fprintf(fp, "0x%X, ", image[y][x]);
		}
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n};\n");
	fclose(fp);
#else
#define UNUSED(x)
	UNUSED(name);
	printf("#define BMP_width %d\n", WIDTH);
	printf("#define BMP_height %d\n", HEIGHT);
	printf("static char BMP_bits[] = {\n");
	for(y = 0; y < h; y++) {
		printf("\t");
		for(x = 0; x < w; x++) {
			printf("0x%X, ", image[y][x]);
		}
		printf("\n");
	}
	printf("\n};\n");
#undef UNUSED(x)
#endif
}
/* Program to convert from ttf to C array.
 */
int main(int argc, char **argv)
{
	FT_Matrix matrix;
	FT_GlyphSlot slot;
	FT_Vector pen;
	double angle;
	int target_height;
	int g;

	memset(image, 0, WIDTH*HEIGHT);
	if(argc != 3) {
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
	if((err = FT_Set_Char_Size(face, 0, 50*64, 300, 300))) {
		fprintf(stderr, "Error: %s\n", FT_Error_String(err));
		FT_Done_Face(face);
		FT_Done_FreeType(library);
		exit(1);
	}
	slot = face->glyph;
	target_height = HEIGHT;
	angle = (25.0/360)*3.14159*2; /* use 25 degrees */
	matrix.xx = (FT_Fixed)( cos(angle)*10000L );
	matrix.xy = (FT_Fixed)(-sin(angle)*10000L );
	matrix.yy = (FT_Fixed)( sin(angle)*10000L );
	matrix.yx = (FT_Fixed)( cos(angle)*10000L );
	pen.x = 300*64;
	pen.y = (target_height-200)*64;
	for(g = 0; g < 256; g++) {
		FT_Set_Transform(face, &matrix, &pen);

		err = FT_Load_Char(face, g, FT_LOAD_RENDER);
		if(err) {
			fprintf(stderr, "Warning: %s\n",
				get_error_string(err));
			continue;
		}
		to_bitmap(&slot->bitmap, slot->bitmap_left,
			target_height-slot->bitmap_top);
		pen.x += slot->advance.x;
		pen.y += slot->advance.y;
	}
	out_xbm(argv[2], WIDTH, HEIGHT);
	FT_Done_Face(face);
	FT_Done_FreeType(library);
	return 0;
}
