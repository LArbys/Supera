#include "RawDigitsToImageI.h"
#include <iostream>

#include "art/Framework/Services/Registry/ServiceHandle.h" // for the convenience of includers
// #include "messagefacility/MessageLogger/MessageLogger.h"
//#include "Geometry/Geometry.h" // include this and disaster happens

#include "RawData/RawDigit.h"

namespace larcaffe {

  RawDigitsToImageI::RawDigitsToImageI() {}

  RawDigitsToImageI::~RawDigitsToImageI() {}

  void RawDigitsToImageI::convert( const std::vector<raw::RawDigit>& digitVec, const int chstart, const int chend, Image& aimage ) {

    std::cout << "[RawDigitsToImage] Store ADCs " << std::endl;

    //art::ServiceHandle<geo::Geometry> geom;
  
    // //Image aimage();
    bool setsize = false;

    int nwfms = chend-chstart;
    int nchs = 0;
    int nadcs = 0;
    for(size_t rdIter = 0; rdIter < digitVec.size(); ++rdIter){
      const raw::RawDigit& digits = digitVec.at( rdIter );
      //geo::View_t view = geom->View( digits.Channel() );
      //if ( view==geo::kZ ) {
      if ( true ) {
    	// select collection plane
    	int idx_ch = digits.Channel()-chstart;
    	if ( idx_ch<0 || idx_ch>=nwfms )
    	  continue; // out of range, skip
    	if ( !setsize ) {
    	  // we can set the image size now
    	  aimage.setSize( digits.Samples(), nwfms );
    	  setsize = true;
    	}
    	nchs++;
    	for (unsigned int t=0; t<digits.Samples(); t++) {
    	  aimage.setpixel( t, idx_ch, (float)digits.ADC(t) );
    	  nadcs++;
    	}
      }
    }
    
  }
}
