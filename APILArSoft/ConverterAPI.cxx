
#include "ConverterAPI.h"
#include "Geometry/Geometry.h"
#include "Utilities/DetectorProperties.h"
#include "LArCaffe/larbys.h"

namespace larcaffe {
  namespace supera {

    void ConverterAPI::SetWireRange(int min, int max, unsigned int plane_id)
    {

      art::ServiceHandle<geo::Geometry> geom;
      if(geom->Nplanes() <= plane_id) {
	logger().LOG(::larcaffe::msg::kCRITICAL,__FUNCTION__,__LINE__)
	  << "Plane ID " << plane_id << " (set via Fhicl) is invalid!" << std::endl;
	throw ::larcaffe::larbys();
      }

      const unsigned int nwires = geom->Nwires(plane_id);
      if(min < 0) min = 0;
      if(max < 0) max = nwires - 1;

      if( (min  >= (int)nwires) || (max >= (int)nwires) || (max < min ) ) {
	  
	logger().LOG(::larcaffe::msg::kCRITICAL,__FUNCTION__,__LINE__)
	  << "Logic error: the range (" << min << "," <<max
	  << ") cannot be set for # wires = " << nwires << std::endl;
	throw ::larcaffe::larbys();
      }

      if(_wire_range_v.empty()) {
	_wire_range_v.resize(geom->Nplanes());
	for(size_t plane=0; plane<_wire_range_v.size(); ++plane) {
	  _wire_range_v[plane].first = 0;
	  _wire_range_v[plane].second = geom->Nwires(plane);
	}
      }

      _wire_range_v[plane_id].first  = min;
      _wire_range_v[plane_id].second = max;

    }
    
    void ConverterAPI::SetTimeRange(int min, int max, unsigned int plane_id)
    {

      art::ServiceHandle<geo::Geometry> geom;
      art::ServiceHandle<util::DetectorProperties> detp;
      if(geom->Nplanes() <= plane_id) {
	logger().LOG(::larcaffe::msg::kCRITICAL,__FUNCTION__,__LINE__)
	  << "Plane ID " << plane_id << " (set via Fhicl) is invalid!" << std::endl;
	throw ::larcaffe::larbys();
      }

      const unsigned int nticks = detp->NumberTimeSamples();
      if(min < 0) min = 0;
      if(max < 0) max = nticks - 1;

      if( (min  >= (int)nticks) || (max >= (int)nticks) || (max < min ) ) {
	logger().LOG(::larcaffe::msg::kCRITICAL,__FUNCTION__,__LINE__)
	  << "Logic error: the range (" << min << "," <<max
	  << ") cannot be set for # ticks = " << nticks << std::endl;
	throw ::larcaffe::larbys();
      }

      if(_time_range_v.empty()) {
	_time_range_v.resize(geom->Nplanes());
	for(size_t plane=0; plane<_time_range_v.size(); ++plane) {
	  _time_range_v[plane].first = 0;
	  _time_range_v[plane].second = nticks;
	}
      }
      _time_range_v[plane_id].first  = min;
      _time_range_v[plane_id].second = max;

    }

    bool ConverterAPI::InRange(const ::geo::WireID& wid) const {

      auto& wire_range = _wire_range_v[wid.Plane];

      return ( wire_range.first  <= wid.Wire && 
	       wire_range.second >= wid.Wire );
    }

    bool ConverterAPI::InRange(const ::geo::WireID& wid, const unsigned int time) const {

      if(!InRange(wid)) return false;

      auto& time_range = _time_range_v[wid.Plane];

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

	auto& time_range = _time_range_v[wire_id.Plane];
	
	if(logger().debug())
	  logger().LOG(::larcaffe::msg::kDEBUG,__FUNCTION__,__LINE__)
	    << "Filling for wire " << wire_id.Wire 
	    << " length " << wf.ADCs().size()
	    << " index " << time_range.first << " => " << time_range.second << std::endl;
	
	if(wf.NADC() > time_range.second) {

	  conv.copy_data( (unsigned int)(wire_id.Wire),
			  (std::vector<short>)(wf.ADCs()),
			  time_range.first,
			  time_range.second - time_range.first + 1,
			  0, 0 );
	  
	}else{

	  logger().LOG(::larcaffe::msg::kWARNING,__FUNCTION__,__LINE__)
	    << "Found an waveform length " << wf.NADC() 
	    << " which is shorter than set limit max " << time_range.second 
	    << std::endl;
	  
	  conv.copy_data( (unsigned int)(wire_id.Wire),
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

