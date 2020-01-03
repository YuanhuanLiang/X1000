#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#include <utils/png_decode.h>
#include <utils/log.h>
#include <utils/common.h>
#include <graphics/gr_drawer.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <fb/fb_manager.h>
#include <ft2build.h>
#include FT_FREETYPE_H
int number;
unsigned int color_array[6]={
    0x0000FF , /* blue */
    0x00FF00 , /* green */
    0x00FFFF ,/* blackish green  */
    0xFF0000 ,/* red */
    0xFF00FF ,/* carmine */
    0xFFFF00 ,/* yellow */
    };
static struct fb_manager* fb_manager;

unsigned char *hzkmem;
unsigned char *fbmem;
unsigned int line_width;
unsigned int pixel_width;
static uint32_t fb_width;
static uint32_t fb_height;
static uint32_t fb_bits_per_pixel;

void lcd_put_pixel( int x, int y, unsigned int color )
{
    unsigned char *pen_8 = fbmem +y*line_width + x*pixel_width;
    unsigned short *pen_16;
    unsigned short *pen_32;
    unsigned char red,green,blue;
    pen_16 = (unsigned short *)pen_8;
    pen_32 = (unsigned short *)pen_8;

    switch (pixel_width * 8) {
    case 8:
        *pen_8 = color;
        break;

    case 16:
        /* 565 */
        red   = (color>>16) & 0xff;
        green = (color>>8)  & 0xff;
        blue  = (color>>0)  & 0xff;
        color = ((red>>3)<<11) | ((green>>2)<<5) | ((blue>>3));
        if (color != 0){
            red   = (color_array[number] >> 16) & 0xff;
            green = (color_array[number] >> 8) & 0xff;
            blue  = (color_array[number] >> 0) & 0xff;
            color = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
        }
        *pen_16 = color;
        break;

    case 32:
        *pen_32 = color;
        break;
    default:
        printf("can't support %ddpp\n", pixel_width*8 );
        break;
    }
}

void draw_bitmap( FT_Bitmap*  bitmap, FT_Int x, FT_Int y)
{
    FT_Int  i, j, p, q;
    FT_Int  x_max = x + bitmap->width;
    FT_Int  y_max = y + bitmap->rows;

    for ( i = x, p = 0; i < x_max; i++, p++ ) {
        for ( j = y, q = 0; j < y_max; j++, q++ ) {
            if ( i < 0 || j < 0 || i >= fb_width || j >= fb_height)
                continue;
            lcd_put_pixel(i, j, bitmap->buffer[q * bitmap->width + p]);
        }
    }
}


int main(int argc, char**  argv )
{
    FT_Library    library;
    FT_Face       face;

    FT_GlyphSlot  slot;
    FT_Matrix     matrix;
    FT_Vector     pen;
    FT_Error      error;

    char*         filename;
    double        angle;
    int           n, num_chars;

    wchar_t chinese_str[100] = L"深圳君正";
    fb_manager = get_fb_manager();
    if (fb_manager->init() < 0) {
        fb_manager = NULL;
        return -1;
    }

    fbmem = fb_manager->get_fbmem();
    fb_width = fb_manager->get_screen_width();
    fb_height = fb_manager->get_screen_height();
    fb_bits_per_pixel = fb_manager->get_bits_per_pixel();
    line_width = fb_width * fb_bits_per_pixel / 8;
    pixel_width = fb_bits_per_pixel / 8;

    if (argc != 3) {
        fprintf ( stderr, "usage: %s <font path> <font color(0-5)>\n", argv[0] );
        printf("%s \"/usr/firmware/freetype/FZLTCXHJW.TTF\"  1\n",argv[0] );
        exit( 1 );
    }

    filename      = argv[1];
    number = atol(argv[2]);
    num_chars     = 4;
    angle         = ( 0.0 / 360 ) * 3.14159 * 2;
    error = FT_Init_FreeType( &library );
    if (error){
        return -1;
    }
    error = FT_New_Face( library, filename, 0, &face );
    if (error){
        return -1;
    }
    error = FT_Set_Pixel_Sizes(face, 0, 24);
    if (error){
        return -1;
    }
    slot = face->glyph;
    /* set up matrix */
    matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
    matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
    matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L );
    matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );
    pen.x = 0;
    pen.y = 0;
    for ( n = 0; n < num_chars; n++ ) {
        /* set transformation */
        FT_Set_Transform( face, &matrix, &pen );

        /* load glyph image into the slot (erase previous one) */
        error = FT_Load_Char( face, chinese_str[n], FT_LOAD_RENDER );
        if ( error )
          continue;/* ignore errors */
        draw_bitmap( &slot->bitmap,pen.x, pen.y);
        pen.x += slot->advance.x>>6;
        pen.y += slot->advance.y>>6;
    }
    fb_manager->display();
    FT_Done_Face    ( face );
    FT_Done_FreeType( library );
    return 0;
}



