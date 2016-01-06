#ifndef __Image__
#define __Image__

namespace larcaffe {

  class Image {
    
  public:
    // height = rows
    // width  = columns
    Image();
    Image( int height, int width );
    virtual ~Image();

    void setSize( int height, int width );
    int height() { return fHeight; };
    int width() { return fWidth; };
    float pixel( int h, int w );
    void setpixel( int h, int w, float value );
    bool isInBounds( int h, int w );

  private:
    int fHeight;
    int fWidth;
    float** fPixels;

    void clear();
    void alloc(int height, int width );

  };

}

#endif
