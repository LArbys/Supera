/**
 * \file ImageMeta.h
 *
 * \ingroup LArCV
 * 
 * \brief Class def header for a class ImageMeta
 *
 * @author kazuhiro
 */

/** \addtogroup LArCV

    @{*/
#ifndef __LARCV_IMAGEMETA_H__
#define __LARCV_IMAGEMETA_H__

#include <iostream>
#ifndef __CINT__
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#endif
#include "LArCV/larbys.h"
#include "LArCV/LArCVTypes.h"

namespace larcv {

  class Image2D;
  /**
     \class ImageMeta
     A simple class to store image's meta data including\n
     0) origin (left-bottom corner of the picture) absolute coordinate \n
     1) horizontal and vertical size (width and height) in double precision \n
     2) number of horizontal and vertical pixels \n
     It is meant to be associated with a specific cv::Mat or larcv::Image2D object \n
     (where the latter class contains ImageMeta as an attribute). For cv::Mat, there \n
     is a function ImageMeta::update to constantly update vertical/horizontal # pixels \n
     as it may change in the course of matrix operation.
  */
  class ImageMeta{

    friend class Image2D;

  public:
    
    /// Default constructor: width, height, and origin coordinate won't be modifiable 
    ImageMeta(const double width=0., const double height=0.,
	      const size_t width_npixel=0., const size_t height_npixel=0,
	      const double origin_x=0., const double origin_y=0.,
	      const size_t plane=::larcv::kINVALID_SIZE)
      : _origin(origin_x,origin_y)
      , _width(width)
      , _height(height)
      , _plane(plane)
    {
      if( width  < 0. ) throw larbys("Width must be a positive floating point!");
      if( height < 0. ) throw larbys("Height must be a positive floating point!");
      update(width_npixel,height_npixel);
    }
    
    /// Default destructor
    ~ImageMeta(){}

    const Point2D& origin   () const { return _origin; }
    size_t plane            () const { return _plane;  }
    double width            () const { return _width;  }
    double height           () const { return _height; }
    size_t num_pixel_row    () const { return _width_npixel;      }
    size_t num_pixel_column () const { return _height_npixel;     }
    double pixel_width      () const { return (_width_npixel ? _width / _width_npixel : 0.);     } 
    double pixel_height     () const { return (_height_npixel ? _height / _height_npixel : 0.);   }
    /// Provide relative horizontal coordinate of the center of a specified pixel row
    double pixel_relative_x(size_t w) const { return pixel_width  () * ((double)w + 0.5); }
    /// Provide relative vertical coordinate of the center of a specified pixel column
    double pixel_relative_y(size_t h) const { return pixel_height () * ((double)h + 0.5); }
    /// Provide absol-pppppppute horizontal coordinate of the center of a specified pixel row
    double pixel_x  (size_t w) const { return _origin.x + pixel_relative_x(w); }
    /// Provide absolute vertical coordinate of the center of a specified pixel row
    double pixel_y  (size_t h) const { return _origin.y + pixel_relative_y(h); }
    /// Change # of vertical/horizontal pixels in meta data
    void update(const size_t width_npixel, const size_t height_npixel)
    {
      //if( !width_npixel  ) throw larbys("Number of pixels must be non-zero (width)");
      //if( !height_npixel ) throw larbys("Number of pixels must be non-zero (height)");
      _width_npixel  = width_npixel;
      _height_npixel = height_npixel;
    }

    #ifndef __CINT__
    /// Change # of vertical/horizontal pixels in meta data with cv::Mat as an input
    void update(const ::cv::Mat& mat)
    { update(mat.rows,mat.cols); }
    /// Provide AABox

    ::cv::Rect rectangle(const ImageMeta& meta,bool for_imshow=false) const
    {
      double x_min = meta.origin().x;
      double x_max = meta.origin().x + meta.width();
      double y_min = meta.origin().y;
      double y_max = meta.origin().y + meta.height();

      double bb_x_min = origin().x < x_min ? x_min : origin().x;
      double bb_x_max = (origin().x + width()) > x_max ? x_max : (origin().x + width());
      double bb_y_min = origin().y < y_min ? y_min : origin().y;
      double bb_y_max = (origin().y + height()) > y_max ? y_max : (origin().y + height());

      std::cout<<"Image bounds: " << std::endl
	       <<"("<<x_min<<","<<y_min<<") "
	       <<"("<<x_min<<","<<y_max<<") "
	       <<"("<<x_max<<","<<y_max<<") "
	       <<"("<<x_max<<","<<y_min<<") "
	       << std::endl;

      std::cout<<"BB bounds: " << std::endl
	       <<"("<<bb_x_min<<","<<bb_y_min<<") "
	       <<"("<<bb_x_min<<","<<bb_y_max<<") "
	       <<"("<<bb_x_max<<","<<bb_y_max<<") "
	       <<"("<<bb_x_max<<","<<bb_y_min<<") "
	       << std::endl;

      double res_x_min = (bb_x_min - meta.origin().x) / meta.pixel_width();
      double res_y_min = (bb_y_min - meta.origin().y) / meta.pixel_height();
      double res_x_size = (bb_x_max - bb_x_min) / meta.pixel_width();
      double res_y_size = (bb_y_max - bb_y_min) / meta.pixel_height();

      std::cout<<"Combined bounds: " << std::endl
	       <<"("<<res_x_min<<","<<res_y_min<<") "
	       <<"("<<res_x_min<<","<<res_y_min + res_y_size<<") "
	       <<"("<<res_x_min + res_x_size<<","<<res_y_min + res_y_size<<") "
	       <<"("<<res_x_min + res_x_size<<","<<res_y_min<<") "
	       << std::endl;
      /*
      auto rec = ::cv::Rect( res_x_min, res_y_min, res_x_size, res_y_size );
      std::cout<<"("<<rec.tl().x<<","<<rec.tl().y<<") ("<<rec.br().x<<","<<rec.br().y<<")"<<std::endl;
      return rec;
      */
      
      if(!for_imshow) return ::cv::Rect( res_x_min, res_y_min, res_x_size, res_y_size );
      else {
	// x becomes y
	// y becomes x then inverted (i.e. y_min = width - y_max)
	/*
	res_y_min = res_x_min;
	res_x_min = (meta.height() - (bb_y_max - meta.origin().y)) / meta.pixel_height(); // x_min
	std::swap(res_x_size,res_y_size);
	return ::cv::Rect( res_x_min, res_y_min, res_x_size, res_y_size );
	*/
	return ::cv::Rect(res_x_min, meta.height() - (bb_y_max - meta.origin().y) / meta.pixel_height(),
			  res_x_size, res_y_size);
      }

    }
    #endif

  protected:

    larcv::Point2D _origin; ///< Absolute coordinate of the left bottom corner of an image
    double _width;          ///< Horizontal size of an image in double floating precision (in original coordinate unit size)
    double _height;         ///< Vertical size of an image in double floating precision (in original coordinate unit size)
    size_t _width_npixel;   ///< # of pixels in horizontal axis
    size_t _height_npixel;  ///< # of pixels in vertical axis
    size_t _plane;          ///< unique plane ID number
  };

}

#endif
/** @} */ // end of doxygen group 

