/**
 * \file Image2D.h
 *
 * \ingroup LArCV
 * 
 * \brief Class def header for an image data holder
 *
 * @author tmw, kazu
 */

/** \addtogroup LArCV

    @{*/

#ifndef __LARCV_Image2D_H__
#define __LARCV_Image2D_H__

#include <vector>
#include <cstdlib>
#include "ImageMeta.h"
#ifndef __CINT__
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#endif
namespace larcv {

  /**
     \class Image2D
     Meant to be a storage class for an image into a ROOT file. Not yet implemented (don't bother reading!)
  */
  class Image2D {
  //class Image2D : public std::vector<float> {
    
  public:
    
    Image2D(size_t width_npixel=0, size_t height_npixel=0);
    Image2D(const ImageMeta&);
    Image2D(const Image2D&);
    Image2D(const std::string img_file, bool as_imshow=false);
    
#ifndef __CINT__
    Image2D(const cv::Mat&);
#endif

    virtual ~Image2D(){}

    enum CompressionModes_t { kSum, kAverage, kMaxPool};

    void imread(const std::string file_name, bool as_imshow=false);
    void imwrite(const std::string file_name) const;
    void imwrite(const std::string file_name,const std::vector<larcv::ImageMeta>& bbs) const;
    void imshow(const std::string frame_name) const;
    void imshow(const std::string frame_name,const std::vector<larcv::ImageMeta>& bbs) const;
    const ImageMeta& meta_data() const { return _meta; }
    unsigned int height() const { return _meta._height_npixel; }
    unsigned int width()  const { return _meta._width_npixel;  }
    float pixel(size_t w, size_t h) const;
    std::vector<float> copy_compress(size_t height, size_t width, CompressionModes_t mode=kSum) const;
    size_t index( size_t w, size_t h ) const;

    void copy(size_t w, size_t h, const float* src, size_t num_pixel);
    void copy(size_t w, size_t h, const std::vector<float>& src, size_t num_pixel=0);
    void copy(size_t w, size_t h, const short* src, size_t num_pixel);
    void copy(size_t w, size_t h, const std::vector<short>& src, size_t num_pixel=0);

    const std::vector<float>& as_vector() const 
    { return _img; }

    #ifndef __CINT__
    cv::Mat as_mat(bool for_imshow=false) const;
    #endif

    void resize( size_t width_npixel, size_t height_npixel );
    void set_pixel( size_t w, size_t h, float value );
    void paint(float value);
    void clear_data();
    void compress(size_t width_npixel, size_t height_npixel, CompressionModes_t mode=kSum);

  private:
    std::vector<float> _img;
    ImageMeta _meta;
    void clear();
  };

}

#endif
/** @} */ // end of doxygen group 
