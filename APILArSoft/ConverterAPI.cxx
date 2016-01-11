
#include "ConverterAPI.h"
#include "Geometry/Geometry.h"
#include "Utilities/DetectorProperties.h"
#include "LArCaffe/larbys.h"
#include "LArCaffe/LArCaffeUtils.h"
namespace larcaffe {
  namespace supera {

    void ConverterAPI::SetRange(int min, int max, unsigned int plane_id, unsigned int compression_factor)
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
	_compression_factor_v.resize(geom->Nplanes(),1);
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

      if(logger().debug())
	logger().LOG(::larcaffe::msg::kDEBUG,__FUNCTION__,__LINE__) 
	  << "set range " << min << " => " << max << " @ plane " << plane_id << std::endl;
      
      _range_v[plane_id].first  = min;
      _range_v[plane_id].second = max;

      if(plane_id < geom->Nplanes() && compression_factor)

	_compression_factor_v[plane_id] = compression_factor;

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
			    larcaffe::supera::converter_base& conv )
    {
      
      art::ServiceHandle<geo::Geometry> geom;

      if(_img_v.empty()) _img_v.resize(geom->Nplanes());

      for(size_t plane=0; plane<geom->Nplanes(); ++plane) {

	auto const& wire_comp = _compression_factor_v[plane];
	auto const& time_comp = _compression_factor_v.back();
	auto const& wire_range = _range_v[plane];
	auto const& time_range = _range_v.back();

	if(wire_comp < 2 && time_comp < 2) continue;

	auto& img = _img_v[plane];

	img.resize( (time_range.second - time_range.first + 1), (wire_range.second - wire_range.first + 1) );

	img.clear_data();
      }

      for(auto const& wf : digit_v) {
	
	unsigned int ch = wf.Channel();
	
	auto const wire_id = geom->ChannelToWire(ch).front();

	auto const& wire_range = _range_v[wire_id.Plane];

	auto const& time_range = _range_v.back();

	if(!InRange(wire_id)) continue;

	if(wire_range.first == wire_range.second) continue;

	if(wire_range.first == wire_range.second && time_range.first == time_range.second) continue;

	if(logger().debug())
	  logger().LOG(::larcaffe::msg::kDEBUG,__FUNCTION__,__LINE__)
	    << "Filling for wire " << wire_id.Wire 
	    << " length " << wf.ADCs().size()
	    << " index " << time_range.first << " => " << time_range.second << std::endl;

	if(wf.NADC() <= time_range.second) {

	  logger().LOG(::larcaffe::msg::kERROR,__FUNCTION__,__LINE__)
	    << "Found an waveform length " << wf.NADC() 
	    << " which is shorter than set limit max " << time_range.second 
	    << std::endl;
	  
	  /*
	  conv.copy_data( (unsigned int)(wire_id.Wire) - wire_range.first,
			  (std::vector<short>)(wf.ADCs()),
			  time_range.first,
			  wf.NADC() - time_range.first,
			  0, 0 );
	  */

	  throw larbys();
	}

	auto const& wire_comp = _compression_factor_v[wire_id.Plane];
	auto const& time_comp = _compression_factor_v.back();

	if(wire_comp < 2 && time_comp < 2) {

	  if(logger().debug()) 
	    logger().LOG(msg::kDEBUG,__FUNCTION__,__LINE__)
	      << "Saving into image @ " << wire_id.Wire - wire_range.first << " column size " << time_range.second - time_range.first + 1
	      << " into datum " << conv.data().height() << " : " << conv.data().width() << std::endl;

	  conv.copy_data( (unsigned int)(wire_id.Wire - wire_range.first),
			  (std::vector<short>)(wf.ADCs()),
			  time_range.first,
			  time_range.second - time_range.first + 1,
			  0, 0 );
	}else {
	  
	  auto& img = _img_v[wire_id.Plane];

	  /*
	  img.copy(0,
		   wire_id.Wire - wire_range.first,
		   &(wf.ADCs()[time_range.first]),
		   time_range.second - time_range.first);
	  */
	  img.copy(0,
		   wire_id.Wire - wire_range.first,
		   (short*)(&(wf.ADCs()[time_range.first])),
		   time_range.second - time_range.first);
	  
	}
      }

      for(size_t plane=0; plane < geom->Nplanes(); ++plane) {

	auto& img = _img_v[plane];

	if(!img.as_vector().size()) continue;
	auto const& wire_comp = _compression_factor_v[plane];
	auto const& time_comp = _compression_factor_v.back();

	auto const& wire_range = _range_v[plane];
	if(wire_range.first==wire_range.second) continue;

	if(wire_comp || time_comp)

	  img.compress(img.height()/time_comp,
		       img.width()/wire_comp);
	
	std::cout << "Saving image (" << img.height() << "," << img.width() 
		  << ") after compression (" << time_comp << "," << wire_comp << std::endl;
	for(size_t w=0; w<img.width(); ++w) {
	  
	  conv.copy_data((unsigned int)w,
			 img.as_vector(),
			 img.index(w,0), img.height(),
			 0, 0);
	  
	}
      }
    }
    
    void ConverterAPI::Copy(const std::vector<recob::Wire>&, larcaffe::supera::converter_base&)
    {}
    
    void ConverterAPI::Copy(const std::vector<recob::Hit>&, larcaffe::supera::converter_base&)
    {}

    //void ConverterAPI::
    
  }
}

