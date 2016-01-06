#include "MCImage.h"
#include <cstddef>

namespace larcaffe {


  MCImage::MCImage() {
    precompressed_collection = NULL;
    compressed_collection = NULL;
  }

  MCImage::~MCImage() {
    if ( compressed_collection )
      delete compressed_collection;
    if ( precompressed_collection )
      delete precompressed_collection;
  }
  
}
