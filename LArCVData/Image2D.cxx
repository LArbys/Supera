#include "LArCV/larbys.h"
#include "Image2D.h"
#include <iostream>
#include <string.h>
#include <stdio.h>
namespace larcv {

  Image2D::Image2D(size_t width_npixel, size_t height_npixel )
    : _img(height_npixel*width_npixel,0.)
    , _meta(width_npixel,height_npixel,width_npixel,height_npixel,0.,0.)
  {import_array();}

  Image2D::Image2D(const ImageMeta& meta)
    : _img(meta.num_pixel_row()*meta.num_pixel_column(),0.)
    , _meta(meta)
  {import_array();}

  Image2D::Image2D(const Image2D& rhs) 
    : _img(rhs._img)
    , _meta(rhs._meta)
  {import_array();}

  Image2D::Image2D(const std::string image_file)
    : _img(0,0.)
    , _meta(0.,0.,1,1,0.,0.)
  { imread(image_file); import_array();}

  void Image2D::imread(const std::string file_name)
  {
    ::cv::Mat image;
    image = ::cv::imread(file_name.c_str(), CV_LOAD_IMAGE_COLOR);

    _img.resize(image.cols * image.rows);

    _meta = ImageMeta(image.rows,image.cols,image.rows, image.cols, 0., 0.);
      
    unsigned char* px_ptr = (unsigned char*)image.data;
    int cn = image.channels();
    
    for(int i=0;i<image.rows;i++) {
      for (int j=0;j<image.cols;j++) {
	float q = 0;
	q += (float)(px_ptr[i*image.cols*cn + j*cn + 0]);               //B
	q += (float)(px_ptr[i*image.cols*cn + j*cn + 1]) * 256.;        //G
	q += (float)(px_ptr[i*image.cols*cn + j*cn + 2]) * 256. * 256.; //R
	set_pixel(i,j,q);
      }
    }
  }

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
    if ( h >= _meta._height_npixel || w >= _meta._width_npixel ) {
      std::cout<<w<<" : "<<h<<std::endl;
      throw larbys("Out-of-bound pixel set request!");
    }
    _img[w*_meta._height_npixel + h] = value;
  }

  void Image2D::paint(float value)
  { for(auto& v : _img) v=value; }
  
  float Image2D::pixel( size_t w, size_t h ) const 
  { return _img[index(w,h)]; }

  size_t Image2D::index( size_t w, size_t h ) const {
    
    if ( w >= width() || h >= height() ) throw larbys("Invalid pixel index queried");
    
    return ( w * _meta._height_npixel + h );
  }

  void Image2D::copy(size_t w, size_t h, const float* src, size_t num_pixel) 
  { 
    const size_t idx = index(w,h);
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
    const size_t idx = index(w,h);
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

  void Image2D::imshow(const std::string frame_name) const{
    ::cv::imshow(frame_name.c_str(),as_mat());
  }

  cv::Mat Image2D::as_mat() const
  {
    cv::Mat img(height(),width(),CV_8UC3);
    
    unsigned char* px_ptr = (unsigned char*)img.data;
    int cn = img.channels();
    
    for(int i=0;i<height();i++) {
      for (int j=0;j<width();j++) {
	
	float q = pixel(j,height()-i-1);
	px_ptr[i*img.cols*cn + j*cn + 0] = (unsigned char)(((int)(q+0.5)));
	px_ptr[i*img.cols*cn + j*cn + 1] = (unsigned char)(((int)(q+0.5))/256);
	px_ptr[i*img.cols*cn + j*cn + 2] = (unsigned char)(((int)(q+0.5))/256/256);
      }
    }
    return img;
  }

  PyObject* Image2D::as_ndarray() const
  {
    int* dim_data = new int[2];
    dim_data[0] = width();
    dim_data[1] = height();
    
    return PyArray_FromDimsAndData( 2, dim_data, NPY_FLOAT, (char*) &(_img[0]));
  }
}
