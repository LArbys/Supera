
#ifndef __LARCV_PRODUCTMAP_H__
#define __LARCV_PRODUCTMAP_H__

#include <string>
#include "larbys.h"

namespace larcv {

  template<class T> std::string ProductName();  

  class Image2D;
  template<> std::string ProductName<larcv::Image2D>();

  class ImageMeta;  
  template<> std::string ProductName<larcv::ImageMeta>();

  //template<class T> std::string ProductName()
  //{ throw larbys("Unsupported type!"); return "noname"; }

}
#endif
