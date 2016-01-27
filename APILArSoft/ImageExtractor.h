#ifndef __IMAGEEXTRACTOR__
#define __IMAGEEXTRACTOR__

// Class takes a Range, RawDigits, and outputs an larcaffe::Image
// This is a simplification of ConverterAPI.  This is a placeholder until ConverterAPI is cleaned up.

#include "LArCaffe/larcaffe_base.h"
#include "ConverterAPI.h" // for definition of Range_t ... (needs massive reorg)
#include "RawData/RawDigit.h"

namespace larcaffe {

  namespace supera {

    class ImageExtractor : public larcaffe::larcaffe_base {
      // larcaffe_base gives us access to logger

    public:
      
      ImageExtractor() {};
      ~ImageExtractor() {};

      larcaffe::Image Extract( int planeid, const Range_t& wirerange, const Range_t& timerange, const std::vector<raw::RawDigit>& rawdigits );
      
    };

  }
}

#endif
