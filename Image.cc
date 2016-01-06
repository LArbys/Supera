#include "Image.h"
#include <iostream>

namespace larcaffe {

  Image::Image( int height, int width ) {
    fPixels = NULL;
    setSize( height, width );
  }

  Image::Image() {
    fPixels = NULL;
  }

  Image::~Image() {
    clear();
  }

  void Image::setSize( int height, int width ) {
    clear();
    fHeight = height;
    fWidth = width;
    alloc( fHeight, fWidth );
  }
  
  void Image::clear() {
    if ( !fPixels )
      return;
    for (int h=0; h<fHeight; h++)
      delete [] fPixels[h];
    delete [] fPixels;
    fPixels = NULL;
  }

  void Image::alloc( int height, int width ) {
    fPixels = new float*[height];
    for (int h=0; h<height; h++)
      fPixels[h] = new float[width];
  }

  void Image::setpixel( int h, int w, float value ) {
    if ( !isInBounds( h, w ) )
      return;
    fPixels[h][w] = value;
  }

  float Image::pixel( int h, int w ) {
    if ( !isInBounds( h, w ) )
      return 0;

    return fPixels[h][w];
  }
  
  bool Image::isInBounds( int h, int w ) {
    if ( (h<0 || h>=fHeight)
	 || (w<0 || w>=fWidth) ) {
      std::cout << "[Image] request pixel (" << h << ", " << w << ") is out of range of image (" << fHeight << ", " << fWidth << ")" << std::endl;
      return false;
    }
    return true;
  }
}
