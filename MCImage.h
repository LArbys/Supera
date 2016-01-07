#ifndef __MCImage__
#define __MCImage__

#include "Image.h"

namespace larcaffe {

  class MCImage {
    
  public:
    // Maybe Cropper acts as MCImage factory. Then we can hide these constructors?
    MCImage();
    virtual ~MCImage();
    
    int interactionmode;
    
    double nu_energy_gev;

    bool unset;

    // we use MCTrack to define a bounding box for the interaction
    double mctrack_boundingbox[4][2];    

    // using the bounding box, we clip out an image of the interaction
    Image precompressed_collection;

    // we will then compress the image (using maxpooling) to bring the image down to a target size (e.g. 247x247)
    Image compressed_collection;

    // right now everything is the collection plane. need to add ability to add planes and such.
    // also need to think hard about framework and how this data type should look. but now just want experience of
    // with how these steps will go

  };

}

#endif
