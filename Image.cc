#include "Image.h"
#include <iostream>

namespace larcaffe {

  Image::Image( int height, int width ) {
    setSize( height, width );
  }

  Image::Image() {
  }

  Image::~Image() {
    clear();
  }

  // // copy constructor
  // Image::Image( Image& source ) {
  //   // deep copy
  //   setSize( source.height(), source.width() );
  //   for (int h=0; h<source.height(); h++) {
  //     for (int w=0; w<source.width(); w++) {
  // 	setpixel( h, w, source.pixel(h,w) );
  //     }
  //   }
  // }

  // // assignment operator
  // Image& Image::operator=( Image& source ) {
  //   // deep copy
  //   setSize( source.height(), source.width() );
  //   for (int h=0; h<source.height(); h++) {
  //     for (int w=0; w<source.width(); w++) {
  // 	setpixel( h, w, source.pixel(h,w) );
  //     }
  //   }
  //   return *this;
  // }

  void Image::setSize( int height, int width ) {
    clear();
    fHeight = height;
    fWidth = width;
    alloc( fHeight, fWidth );
  }
  
  void Image::clear() {
    fPixels.clear();
    fWidth = 0;
    fHeight = 0;
    // if ( !fPixels )
    //   return;
    // for (int h=0; h<fHeight; h++)
    //   delete [] fPixels[h];
    // delete [] fPixels;
    // fPixels = NULL;
    // std::cout << "[Image (" << this << ")] Cleared image memory " << std::endl;
  }

  void Image::alloc( int height, int width ) {
    // fPixels = new float*[height];
    fPixels.resize( height );
    for (int h=0; h<height; h++)
      fPixels.at(h).resize(width);
    //fPixels[h] = new float[width];
    std::cout << "[Image (" << this << ")] Allocated space for image H: " << height << " W: " << width << std::endl;
  }

  void Image::setpixel( int h, int w, float value ) {
    if ( !isInBounds( h, w ) )
      return;
    fPixels.at(h).at(w) = value;
  }
  
  float Image::pixel( int h, int w ) {
    if ( !isInBounds( h, w ) )
      return 0;

    return fPixels.at(h).at(w);
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
