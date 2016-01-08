
#include "ConverterAPI.h"
#include "Geometry/Geometry.h"
#include "Utilities/DetectorProperties.h"
#include "LArCaffe/larbys.h"

namespace larcaffe {
  namespace supera {

    void ConverterAPI::SetRange(int min, int max, unsigned int plane_id)
    {

      art::ServiceHandle<geo::Geometry> geom;
      art::ServiceHandle<util::DetectorProperties> detp;
      if(geom->Nplanes() < plane_id) {
	logger().LOG(::larcaffe::msg::kCRITICAL,__FUNCTION__,__LINE__)
	  << "Plane ID " << plane_id << " (set via Fhicl) is invalid!" << std::endl;
	throw ::larcaffe::larbys();
      }

      if(_range_v.empty()) {
	_range_v.resize(geom->Nplanes()+1,Range_t(0,0));
	for(size_t plane=0; plane<geom->Nplanes(); ++plane) {
	  _range_v[plane].first  = 0;
	  _range_v[plane].second = geom->Nwires(plane) - 1;
	}
	_range_v[geom->Nplanes()].first=0;
	_range_v[geom->Nplanes()].second=detp->NumberTimeSamples() - 1;
      }

      const unsigned int hard_limit = (plane_id < geom->Nplanes() ? (geom->Nwires(plane_id) - 1)  : (detp->NumberTimeSamples() - 1));

      if(min < 0) min = 0;
      if(max < 0) max = hard_limit;
      
      if( (min  > (int)hard_limit) || (max > (int)hard_limit) || (max < min ) ) {
	
	logger().LOG(::larcaffe::msg::kCRITICAL,__FUNCTION__,__LINE__)
	  << "Logic error: the range (" << min << "," <<max
	  << ") cannot be set for plane " << plane_id << " (max=" << hard_limit << ")!" << std::endl;
	throw ::larcaffe::larbys();
      }

      if(logger().info())
	logger().LOG(::larcaffe::msg::kINFO,__FUNCTION__,__LINE__) 
	  << "set range " << min << " => " << max << " @ plane " << plane_id << std::endl;
      
      _range_v[plane_id].first  = min;
      _range_v[plane_id].second = max;

    }

    bool ConverterAPI::InRange(const ::geo::WireID& wid) const {

      auto& wire_range = _range_v[wid.Plane];

      return ( wire_range.first  <= wid.Wire && 
	       wire_range.second >= wid.Wire );
    }

    bool ConverterAPI::InRange(const ::geo::WireID& wid, const unsigned int time) const {

      if(!InRange(wid)) return false;

      auto& time_range = _range_v.back();

      return ( time_range.first  <= time &&
	       time_range.second >= time );
    }

    void ConverterAPI::Copy(const std::vector<raw::RawDigit>& digit_v, 
			    larcaffe::supera::converter_base& conv) const
    {

      art::ServiceHandle<geo::Geometry> geom;

      for(auto const& wf : digit_v) {
	
	unsigned int ch = wf.Channel();
	
	auto const wire_id = geom->ChannelToWire(ch).front();

	if(!InRange(wire_id)) continue;

	auto const& wire_range = _range_v[wire_id.Plane];

	auto const& time_range = _range_v.back();
	
	if(logger().debug())
	  logger().LOG(::larcaffe::msg::kDEBUG,__FUNCTION__,__LINE__)
	    << "Filling for wire " << wire_id.Wire 
	    << " length " << wf.ADCs().size()
	    << " index " << time_range.first << " => " << time_range.second << std::endl;
	
	if(wf.NADC() > time_range.second) {

	  conv.copy_data( (unsigned int)(wire_id.Wire - wire_range.first),
			  (std::vector<short>)(wf.ADCs()),
			  time_range.first,
			  time_range.second - time_range.first + 1,
			  0, 0 );
	  
	}else{

	  logger().LOG(::larcaffe::msg::kWARNING,__FUNCTION__,__LINE__)
	    << "Found an waveform length " << wf.NADC() 
	    << " which is shorter than set limit max " << time_range.second 
	    << std::endl;
	  
	  conv.copy_data( (unsigned int)(wire_id.Wire) - wire_range.first,
			  (std::vector<short>)(wf.ADCs()),
			  time_range.first,
			  wf.NADC() - time_range.first,
			  0, 0 );      
	}
      }
    }
    
    void ConverterAPI::Copy(const std::vector<recob::Wire>&, larcaffe::supera::converter_base&) const
    {}
    
    void ConverterAPI::Copy(const std::vector<recob::Hit>&, larcaffe::supera::converter_base&) const
    {}
    
  }
}

