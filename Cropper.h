#ifndef __CROPPER__
#define __CROPPER__

#include <vector>

// LArSoft
#include "MCBase/MCTrack.h"

// LArCaffe
#include "Image.h"
#include "MCImage.h"

namespace larcaffe {

  class Cropper {
  public:
    Cropper();
    virtual ~Cropper();
    
    void crop( const std::vector< sim::MCTrack >& mctracks, Image& rawadcimage, std::vector<MCImage>& output  );

    // config parameters
    int targetwidth;
    int targetheight;
    int minpadding_t;
    int minpadding_w;
    

  };

}


#endif
