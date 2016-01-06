// LArSoft to LArCaffe interface
#ifndef __RawDigitsToImageI__
#define __RawDigitsToImageI__

#include <vector>

// LarSoft
#include "RawData/RawDigit.h"

// LArCaffe
#include "Image.h"

namespace larcaffe {
  
  class RawDigitsToImageI {
  public:
    RawDigitsToImageI();
    ~RawDigitsToImageI();
    
    void convert( const std::vector<raw::RawDigit>& digitVecHandle, const int chstart, const int chend, Image& aimage );

  };
  

}


#endif
