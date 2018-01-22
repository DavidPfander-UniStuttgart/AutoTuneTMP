#include <fstream>
#include <string>
using namespace std;

typedef unsigned char byte;
typedef struct  {
  byte red, green, blue;
} RGB_t;

// It is presumed that the image is stored in memory as
//   RGB_t data[ height ][ width ]
// where lines are top to bottom and columns are left to right
// (the same way you view the image on the display)

// The routine makes all the appropriate adjustments to match the TGA format specification.

RGB_t ColorRGB (byte r, byte g, byte b) {
	RGB_t color;
	color.red = r;
	color.green = g;
	color.blue = b;
	return color;
}

bool write_tga( const string& filename, RGB_t* data, unsigned width, unsigned height ) {
  ofstream tgafile( filename.c_str(), ios::binary );
  if (!tgafile) return false;

  // The image header
  byte header[ 18 ] = { 0 };
  header[  2 ] = 2;  // truecolor
  header[ 12 ] =  width        & 0xFF;
  header[ 13 ] = (width  >> 8) & 0xFF;
  header[ 14 ] =  height       & 0xFF;
  header[ 15 ] = (height >> 8) & 0xFF;
  header[ 16 ] = 24;  // bits per pixel

  tgafile.write( (const char*)header, 18 );

  // The image data is stored bottom-to-top, left-to-right
  for (int y = height -1; y >= 0; y--)
  for (uint32_t x = 0; x < width; x++)
    {
    tgafile.put( (char)data[ (y * width) + x ].blue  );
    tgafile.put( (char)data[ (y * width) + x ].green );
    tgafile.put( (char)data[ (y * width) + x ].red   );
    }

  // The file footer. This part is totally optional.
  static const char footer[ 26 ] =
    "\0\0\0\0"  // no extension area
    "\0\0\0\0"  // no developer directory
    "TRUEVISION-XFILE"  // yep, this is a TGA file
    ".";
  tgafile.write( footer, 26 );

  tgafile.close();
  return true;
}
