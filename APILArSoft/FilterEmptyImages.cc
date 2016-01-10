#include "FilterEmptyImages.h"
#include <iostream>

namespace larcaffe {

  namespace supera {

    FilterEmptyImages::FilterEmptyImages() {}

    FilterEmptyImages::~FilterEmptyImages() {}

    void FilterEmptyImages::configure( fhicl::ParameterSet const & p ) {

    }

    bool FilterEmptyImages::doWeKeep( const larcaffe::supera::converter_base&  ) { 
      //std::cout << "[FilterEmptyImages] Test. Do Nothing." << std::endl;
      return true; 
    }

    static FilterEmptyImagesFactory global_FilterEmptyImagesFactory;

  }

}
