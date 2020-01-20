#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H
#include <math.h>

#define WIDTH		640
#define HEIGHT		480
#define BYTEWIDTH	((int)((WIDTH)/8))

/* some definitions */
static unsigned char image[HEIGHT][BYTEWIDTH];
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

	for(i = x, p = 0; i < x_max; i++, p++)
		for(j = y, q = 0; j < y_max; j++, q++) {
			if(i < 0 || j < 0 || i >= WIDTH || j >= HEIGHT)
				continue;
			image[j][i] |= bitmap->buffer[q*bitmap->width+p];
		}
}
/* Draw glyph to bitmap.
 */
static void draw_glyph(unsigned char glyph, FT_Vector *pen, FT_Matrix *matrix)
{
	FT_GlyphSlot slot = face->glyph;
	FT_Set_Transform(face, matrix, pen);
	if((err = FT_Load_Char(face, glyph, FT_LOAD_DEFAULT))) {
		fprintf(stderr, "warning: Glyph 0x%X: %s\n",
			glyph, get_error_string(err));
		return;
	}
	to_bitmap(&slot->bitmap, pen->x, pen->y);
	pen->x += slot->advance.x;
	pen->y += slot->advance.y;
}
/* Make an output file with xbm extension.
 */
static void out_xbm(int w, int h)
{
	int x, y;
	printf("#define BMP_width %d\n", WIDTH);
	printf("#define BMP_height %d\n", HEIGHT);
	printf("static char BMP_bits[] = {\n");
	for(y = 0; y < h; y++) {
		printf("\t");
		for(x = 0; x < w; x++) {
			printf("0x%0X, ", image[y][x]);
		}
		printf("\n");
	}
	printf("\n};\n");
}
/* Program to convert from ttf to C array.
 */
int main(int argc, char **argv)
{
	FT_Matrix matrix;
	FT_Vector pen;
	double angle;
	int g;

	memset(image, 0, BYTEWIDTH*HEIGHT);
	if(argc != 2) {
		fprintf(stderr, "Usage: %s <font.ttf>\n", argv[0]);
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
	if((err = FT_Set_Char_Size(face, 50*64, 0, 100, 0))) {
		fprintf(stderr, "Error: %s\n", FT_Error_String(err));
		FT_Done_Face(face);
		FT_Done_FreeType(library);
		exit(1);
	}
	angle = (25.0/360)*3.14159*2; /* use 25 degrees */
	matrix.xx = (FT_Fixed)( cos(angle)*0x10000L );
	matrix.xy = (FT_Fixed)(-sin(angle)*0x10000L );
	matrix.yy = (FT_Fixed)( sin(angle)*0x10000L );
	matrix.yx = (FT_Fixed)( cos(angle)*0x10000L );
	pen.x = 300 * 64;
	pen.y = (HEIGHT - 200) * 64;
	for(g = 0; g < 256; g++)
		draw_glyph(g, &pen, &matrix);
	out_xbm(BYTEWIDTH, HEIGHT);
	FT_Done_Face(face);
	FT_Done_FreeType(library);
	return 0;
}
