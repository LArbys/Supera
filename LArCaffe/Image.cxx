#include "Image.h"
#include "larbys.h"
#include <iostream>
#include <string.h>
#include <stdio.h>
namespace larcaffe {

  Image::Image(size_t height, size_t width )
    : std::vector<float>(height*width,0.)
    , fHeight(height)
    , fWidth(width)
  {}

  Image::Image(const Image& rhs) 
    : std::vector<float>(rhs)
    , fHeight(rhs.fHeight)
    , fWidth(rhs.fWidth)
  {}

  void Image::resize(size_t height, size_t width ) 
  {
    std::vector<float>::resize(height*width);
    fHeight = height;
    fWidth  = width;
  }
  
  void Image::clear() {
    std::vector<float>::clear();
    fWidth = 0;
    fHeight = 0;
    std::cout << "[Image (" << this << ")] Cleared image memory " << std::endl;
  }

  void Image::clear_data() { for(auto& v : (*this)) v = 0.; }

  void Image::set_pixel( size_t h, size_t w, float value ) {
    if ( h >= fHeight || w >= fWidth ) throw larbys("Out-of-bound pixel set request!");
    (*this)[w*fHeight + h] = value;
  }

  void Image::paint(float value)
  { for(auto& v : (*this)) v=value; }
  
  float Image::pixel( size_t h, size_t w ) const 
  { return (*this)[index(h,w)]; }

  size_t Image::index( size_t h, size_t w ) const {
    
    if ( !isInBounds( h, w ) ) throw larbys("Invalid pixel index queried");
    
    return ( w * fHeight + h );
  }

  void Image::copy(size_t h, size_t w, const float* src, size_t num_pixel) 
  { 
    const size_t idx = index(h,w);
    if(idx+num_pixel > size()) {
      char oops[500];
      sprintf( oops,"memcpy end (%d) exceeds allocated memory (%d)!", idx+num_pixel, size() );
      throw larbys(oops);
    }
    
    memcpy(&((*this)[idx]),src, num_pixel * sizeof(float));

  }

  void Image::copy(size_t h, size_t w, const std::vector<float>& src, size_t num_pixel) 
  {
    if(!num_pixel)
      this->copy(h,w,(float*)(&(src[0])),src.size());
    else if(num_pixel < src.size()) 
      this->copy(h,w,(float*)&src[0],num_pixel);
    else
      throw larbys("Not enough pixel in source!");
  }

  void Image::copy(size_t h, size_t w, const short* src, size_t num_pixel) 
  {
    const size_t idx = index(h,w);

    if(idx+num_pixel > size()) {
      char oops[500];
      sprintf( oops, "memcpy end (%d) exceeds allocated memory (%d) [idx=%d,num_pixel=%d]!", idx+num_pixel, size(), idx, num_pixel );
      throw larbys(oops);
    }
    
    for(size_t i=0; i<num_pixel; ++i) (*this)[idx+i] = (float)src[num_pixel];

  }

  void Image::copy(size_t h, size_t w, const std::vector<short>& src, size_t num_pixel) 
  {
    if(!num_pixel)
      this->copy(h,w,(short*)(&(src[0])),src.size());
    else if(num_pixel < src.size()) 
      this->copy(h,w,(short*)&src[0],num_pixel);
    else
      throw larbys("Not enough pixel in source!");
  }

  Image Image::copy_compress(size_t height, size_t width, CompressionModes_t mode) const
  { 
    if(fHeight % height || fWidth % width) {
      char oops[500];
      sprintf(oops,"Compression only possible if height/width are modular 0 of compression factor! H:%dMOD%d=%d W:%dMOD%d=%d",fHeight,height,fHeight%height,fWidth,width,fWidth%width);
      throw larbys(oops);
    }
    size_t width_factor  = fWidth  / width;
    size_t height_factor = fHeight / height;
    Image result(height,width);

    for(size_t w=0; w<width; ++ w) {
      for(size_t h=0; h<height; ++h) {
	float value = 0;
	//std::cout << w*height << " : " << h << std::endl;
	for(size_t orig_w=w*width_factor; orig_w<(w+1)*width_factor; ++orig_w)
	  for(size_t orig_h=h*height_factor; orig_h<(h+1)*height_factor; ++orig_h) {
	    //std::cout << "    " << (*this)[orig_w * fHeight + orig_h] << " @ " << orig_w * fHeight + orig_h << std::endl;
	    if ( mode!=kMaxPool ) {
	      // for sum and average mode
	      value += (*this)[orig_w * fHeight + orig_h];
	    }
	    else {
	      // maxpool
	      value = ( value<(*this)[orig_w * fHeight + orig_h] ) ? (*this)[orig_w * fHeight + orig_h] : value;
	    }
	  }
	//std::cout<<std::endl;
	result[w*height + h] = value;
	if ( mode==kAverage ) 
	  result[w*height + h] /= (float)(width_factor*height_factor);
      }
    }
    return result;
  }

  void Image::compress(size_t height, size_t width, CompressionModes_t mode)
  { (*this) = copy_compress(height,width,mode); }
  
}
