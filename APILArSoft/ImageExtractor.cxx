#include "ImageExtractor.h"

#include "Geometry/Geometry.h" // LArCore
#include "LArCaffe/larbys.h"   // for exception handling

namespace larcaffe {
  namespace supera {

    larcaffe::Image ImageExtractor::Extract( int planeid, const Range_t& wire_range, const Range_t& time_range, const std::vector<raw::RawDigit>& rawdigits ) {
      
      int nticks = time_range.second-time_range.first+1;
      int nwires = wire_range.second-wire_range.first+1;
      
      // Stuck with LArSoft
      art::ServiceHandle<geo::Geometry> geom;
      
      // The output object
      larcaffe::Image img( nticks, nwires );

      for ( auto const& wf : rawdigits ) {
	unsigned int ch = wf.Channel();
	auto const wire_id = geom->ChannelToWire(ch).front();
	if(wf.NADC() <= time_range.second) {
	  logger().LOG(::larcaffe::msg::kERROR,__FUNCTION__,__LINE__)
	    << "Found an waveform length " << wf.NADC()
	    << " which is shorter than set limit max " << time_range.second
	    << std::endl;
	  throw ::larcaffe::larbys();
	}
	
	if ( (int)wire_id.Plane!=planeid )
	  continue;
	
	bool inrange = (wire_range.first <= wire_id.Wire) && (wire_range.second >= wire_id.Wire);
	//std::cout << "plane " << planeid << ": ch=" << ch << " wire " << wire_id.Wire  << " in range: " << inrange << std::endl;
	if (!inrange )
	  continue;
	
	for (int t=0; t<(int)(time_range.second-time_range.first+1);t++)
	  img.set_pixel( t, wire_id.Wire - wire_range.first, (float)wf.ADCs()[time_range.first+t] );
	
      }//end of wire loop
      
      //check image (for desperate times)
      // std::cout << "[Check Pre-Compressed Image]" << std::endl;
      // for (int t=0; t<(int)(time_range.second-time_range.first+1);t++) {
      //   std::cout << img.pixel( t, 1 ) << " ";
      // }
      // std::cout << std::endl;

      return img;
    }

    larcaffe::Image ImageExtractor::Extract( int planeid, const Range_t& wire_range, const Range_t& time_range, const std::vector<recob::Wire>& wires ) {
      
      int nticks = time_range.second-time_range.first+1;
      int nwires = wire_range.second-wire_range.first+1;

      // Stuck with LArSoft
      art::ServiceHandle<geo::Geometry> geom;

      larcaffe::Image img( nticks, nwires );

      for(auto const& wire : wires) {

	auto const wire_id = geom->ChannelToWire(wire.Channel()).front();
	
	auto const plane = wire_id.Plane;

	if((int)plane != planeid) continue;
	
	bool inrange = (wire_range.first <= wire_id.Wire && wire_range.second >= wire_id.Wire);
	if(!inrange) continue;

	auto const& signalROI = wire.SignalROI();

	for(const auto& range : signalROI.get_ranges()) {
	  
	  auto const& adcs = range.data();

	  for(size_t tick = range.begin_index();
	      time_range.first <= tick && tick <= time_range.second && tick < (range.begin_index() + adcs.size());
	      ++tick)

	    img.set_pixel( tick, wire_id.Wire - wire_range.first, adcs[tick-range.begin_index()]);

	}
      }

      return img;
    }

  }
}
