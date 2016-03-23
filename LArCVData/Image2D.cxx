#include "LArCV/larbys.h"
#include "Image2D.h"
#include <iostream>
#include <string.h>
#include <stdio.h>
namespace larcv {

  Image2D::Image2D(size_t width_npixel, size_t height_npixel )
    : _img(height_npixel*width_npixel,0.)
    , _meta(0.,0.,width_npixel,height_npixel,0.,0.)
  {}

  Image2D::Image2D(const ImageMeta& meta)
    : _img(meta.num_pixel_row()*meta.num_pixel_column(),0.)
    , _meta(meta)
  {}

  /*
  Image2D::Image2D(const Image2D& rhs) 
    : _img(rhs)
    , _meta(rhs._meta)
  {}
  */
  void Image2D::resize(size_t width_npixel, size_t height_npixel ) 
  {
    _img.resize(height_npixel*width_npixel);
    _meta.update(width_npixel,height_npixel);
  }
  
  void Image2D::clear() {
    _img.clear();
    _meta.update(1,1);
    //std::cout << "[Image2D (" << this << ")] Cleared image memory " << std::endl;
  }

  void Image2D::clear_data() { for(auto& v : _img) v = 0.; }

  void Image2D::set_pixel( size_t w, size_t h, float value ) {
    if ( h >= _meta._height_npixel || w >= _meta._width_npixel ) throw larbys("Out-of-bound pixel set request!");
      return;
    _img[w*_meta._height_npixel + h] = value;
  }

  void Image2D::paint(float value)
  { for(auto& v : _img) v=value; }
  
  float Image2D::pixel( size_t w, size_t h ) const 
  { return _img[index(h,w)]; }

  size_t Image2D::index( size_t w, size_t h ) const {
    
    if ( w >= width() || h >= height() ) throw larbys("Invalid pixel index queried");
    
    return ( w * _meta._height_npixel + h );
  }

  void Image2D::copy(size_t w, size_t h, const float* src, size_t num_pixel) 
  { 
    const size_t idx = index(h,w);
    if(idx+num_pixel >= _img.size()) throw larbys("memcpy size exceeds allocated memory!");
    
    memcpy(&(_img[idx]),src, num_pixel * sizeof(float));

  }

  void Image2D::copy(size_t w, size_t h, const std::vector<float>& src, size_t num_pixel) 
  {
    if(!num_pixel)
      this->copy(h,w,(float*)(&(src[0])),src.size());
    else if(num_pixel < src.size()) 
      this->copy(h,w,(float*)&src[0],num_pixel);
    else
      throw larbys("Not enough pixel in source!");
  }

  void Image2D::copy(size_t w, size_t h, const short* src, size_t num_pixel) 
  {
    const size_t idx = index(h,w);
    if(idx+num_pixel >= _img.size()) throw larbys("memcpy size exceeds allocated memory!");
    
    for(size_t i=0; i<num_pixel; ++i) _img[idx+i] = src[num_pixel];

  }

  void Image2D::copy(size_t w, size_t h, const std::vector<short>& src, size_t num_pixel) 
  {
    if(!num_pixel)
      this->copy(h,w,(short*)(&(src[0])),src.size());
    else if(num_pixel < src.size()) 
      this->copy(h,w,(short*)&src[0],num_pixel);
    else
      throw larbys("Not enough pixel in source!");
  }

  std::vector<float> Image2D::copy_compress(size_t height, size_t width, CompressionModes_t mode) const
  { 
    const size_t self_width  = _meta.num_pixel_row();
    const size_t self_height = _meta.num_pixel_column();
    if(self_height % height || self_width % width) {
      char oops[500];
      sprintf(oops,"Compression only possible if height/width are modular 0 of compression factor! H:%zuMOD%zu=%zu W:%zuMOD%zu=%zu",
	      self_height,height,self_height%height,self_width,width,self_width%width);
      throw larbys(oops);
    }
    size_t width_factor  = self_width  / width;
    size_t height_factor = self_height / height;
    std::vector<float> result(height,width);

    for(size_t w=0; w<width; ++ w) {
      for(size_t h=0; h<height; ++h) {
	float value = 0;
	//std::cout << w*height << " : " << h << std::endl;
	for(size_t orig_w=w*width_factor; orig_w<(w+1)*width_factor; ++orig_w)
	  for(size_t orig_h=h*height_factor; orig_h<(h+1)*height_factor; ++orig_h) {
	    //std::cout << "    " << _img[orig_w * self_height + orig_h] << " @ " << orig_w * self_height + orig_h << std::endl;
	    if ( mode!=kMaxPool ) {
	      // for sum and average mode
	      value += _img[orig_w * self_height + orig_h];
	    }
	    else {
	      // maxpool
	      value = ( value<_img[orig_w * self_height + orig_h] ) ? _img[orig_w * self_height + orig_h] : value;
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

  void Image2D::compress(size_t height, size_t width, CompressionModes_t mode)
  { _img = copy_compress(height,width,mode); }

  /*
  cv::Mat Image2D::as_mat() const
  {
    return cv::Mat(width(),height(),CV_32FC1,&(((std::vector<float>)_img)[0]));
  }
  */  
}
