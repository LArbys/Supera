

#include <iostream>

#include "Cropper.h"

#include "Geometry/Geometry.h"
#include "Utilities/TimeService.h"
#include "Utilities/DetectorProperties.h"
#include "Utilities/LArProperties.h"

#include "LArCaffe/larbys.h"

namespace larcaffe {

  namespace supera {
    
    Cropper::Cropper(unsigned int time_padding,
		     unsigned int wire_padding,
		     unsigned int target_width,
		     unsigned int target_height,
		     unsigned int compression_factor)
      : _time_padding(time_padding)
      , _wire_padding(wire_padding)
      , _target_width(target_width)
      , _target_height(target_height)
      , _compression_factor(compression_factor)
    {}
    
    void Cropper::configure(unsigned int time_padding,
			    unsigned int wire_padding,
			    unsigned int target_width,
			    unsigned int target_height,
			    unsigned int compression_factor)
    {
      _time_padding = time_padding;
      _wire_padding = wire_padding;
      _target_width = target_width;
      _target_height = target_height;
      _compression_factor = compression_factor;
      if(logger().info())
	logger().LOG(::larcaffe::msg::kINFO,__FUNCTION__,__LINE__) 
	  << " Target size (w,h) = (" << _target_width << "," << _target_height << ")"
	  << " Padding (time,wire) = (" << _time_padding << "," << _wire_padding << ")"
	  << " Compression factor = " << _compression_factor << std::endl;
    }

    RangeArray_t Cropper::WireTimeBoundary(const std::vector<sim::MCTrack>& mct_v) const
    {
      art::ServiceHandle<geo::Geometry> geom;

      RangeArray_t result(4); // result is 3 planes' wire boundary + time boundary (4 elements)
      
      for(auto& r : result) { r.first = larcaffe::kINVALID_UINT; r.second = 0; }
      
      for(auto const& mct : mct_v) {
	
	auto boundary = WireTimeBoundary(mct);
	
	for(size_t i=0; i<result.size(); ++i) {
	  
	  auto& all = result[i];
	  auto& one = boundary[i];
	  if(all.first  > one.first ) all.first  = one.first;
	  if(all.second < one.second) all.second = one.second;
	  
	}
      }
      
      if(logger().info()) {
	
	for(size_t plane=0; plane <= geom->Nplanes(); ++plane)
	  
	  logger().LOG(msg::kINFO,__FUNCTION__,__LINE__) 
	    << "MCTrack Collection ... Plane " << plane 
	    << " bound " << result[plane].first << " => " << result[plane].second << std::endl;
      }
      
      return result;
    }
    
    RangeArray_t Cropper::WireTimeBoundary(const ::sim::MCTrack& mct) const
    {
      art::ServiceHandle<geo::Geometry> geom;
      art::ServiceHandle<util::LArProperties> larp;
      art::ServiceHandle<util::DetectorProperties> detp;
      art::ServiceHandle<util::TimeService> ts;
      const double drift_velocity = larp->DriftVelocity() * 1.e3; // make it nano-sec/cm
      const int tick_max = detp->NumberTimeSamples();
      double xyz[3] = {0.};
      
      RangeArray_t result(4); // result is 3 planes' wire boundary + time boundary (4 elements)
      
      for(auto& r : result) { r.first = larcaffe::kINVALID_UINT; r.second = 0; }
      
      for(auto& step : mct) {
	
	// Figure out time
	int tick = (unsigned int)(ts->TPCG4Time2Tick(step.T() + (step.X() / drift_velocity))) + 1;

	if(tick < 0 || tick >= tick_max) continue;
	
	auto& trange = result.back();
	if(trange.first  > (unsigned int)tick) trange.first  = tick;
	if(trange.second < (unsigned int)tick) trange.second = tick;
	
	// Figure out wire per plane
	xyz[0] = step.X();
	xyz[1] = step.Y();
	xyz[2] = step.Z();
	for(size_t plane=0; plane < geom->Nplanes(); ++plane) {

	  try{

	    auto wire_id = geom->NearestWireID(xyz, plane);
	  
	    if(result[plane].first  > wire_id.Wire) result[plane].first  = wire_id.Wire;
	    if(result[plane].second < wire_id.Wire) result[plane].second = wire_id.Wire;

	  }
	  catch(...) {continue;}

	}
	//std::cout<<xyz[0]<<" : "<<step.T()<<" ... "<<tick<<std::endl;
      }
      
      for(auto& r : result) 
	
	if(r.first == larcaffe::kINVALID_UINT && r.second == 0) r.first = 0;
      
      if(logger().info()) {
	
	for(size_t plane=0; plane <= geom->Nplanes(); ++plane)
	  
	  logger().LOG(msg::kINFO,__FUNCTION__,__LINE__) 
	    << "Single MCTrack ... Plane " << plane 
	    << " bound " << result[plane].first << " => " << result[plane].second << std::endl;
      }
      
      return result;
    }
    
    RangeArray_t Cropper::WireTimeBoundary( const std::vector<sim::SimChannel>& simch_v) const
    {
      
      art::ServiceHandle<geo::Geometry> geom;
      art::ServiceHandle<util::TimeService> ts;
      art::ServiceHandle<util::DetectorProperties> detp;
      RangeArray_t result(geom->Nplanes()+1);
      for(auto& range : result) { range.first = 0; range.second = 0; }
      
      for(auto const& simch : simch_v) {
	
	auto const ch = simch.Channel();
	
	auto const wid = geom->ChannelToWire(ch).front();
	
	auto& wire_range = result[wid.Plane];
	auto& time_range = result.back();
	
	if(!wire_range.first && !wire_range.second)
	  
	  wire_range.first = wire_range.second = wid.Wire;
	
	else {
	  if(wire_range.first  > wid.Wire) wire_range.first  = wid.Wire;
	  if(wire_range.second < wid.Wire) wire_range.second = wid.Wire;
	}
	
	for(auto const& tdc_ide : simch.TDCIDEMap()) {

	  auto const& tdc = tdc_ide.first;
	  
	  auto const tick = ts->TPCTDC2Tick(tdc);

	  if(tick<0 || tick >= detp->NumberTimeSamples()) continue;
	  
	  if(!time_range.first && !time_range.second)
	    
	    time_range.first = time_range.second = tick;
	  
	  else {
	    if(time_range.first  > tick) time_range.first  = tick;
	    if(time_range.second < tick) time_range.second = tick;
	  }
	}
      }
      
      if(logger().info()) {
	
	for(size_t plane=0; plane <= geom->Nplanes(); ++plane)
	  
	  logger().LOG(msg::kINFO,__FUNCTION__,__LINE__) 
	    << "Plane " << plane 
	    << " bound " << result[plane].first << " => " << result[plane].second << std::endl;
      }

      return result;
    }
    
    Range_t Cropper::Format(const Range_t& range, unsigned short plane_id) const
    {
      art::ServiceHandle<util::DetectorProperties> detp;
      art::ServiceHandle<geo::Geometry> geom;
      if(range.second < range.first) {
	logger().LOG(msg::kERROR,__FUNCTION__,__LINE__)
	  << "Inconsistent boundary given: max (" << range.second << ") < min (" << range.first << ") !" << std::endl;
	throw larbys();
      }
      if(plane_id > geom->Nplanes()) {
	logger().LOG(msg::kERROR,__FUNCTION__,__LINE__)
	  << "Invalid plane ID (" << plane_id << ") ... must be <= " << geom->Nplanes() << std::endl;
	throw larbys();
      }
      
      const bool wire = plane_id < geom->Nplanes();
      const int target_width = ( wire ? _target_width : _target_height );
      const int padding = ( wire ? _wire_padding : _time_padding );
      const int max = ( wire ? geom->Nwires(plane_id) : detp->NumberTimeSamples());

      if(target_width > max) {
        logger().LOG(msg::kERROR,__FUNCTION__,__LINE__)
	  << "Image size (" << target_width << ") too large for plane " << plane_id << " (only goes 0=>" << max << ")!" << std::endl;
	throw larbys();
      }
      if(_compression_factor && (int)(target_width * _compression_factor) > max) {
        logger().LOG(msg::kERROR,__FUNCTION__,__LINE__)
	  << "Image size (" << target_width * _compression_factor << ") too large for plane " << plane_id << " (only goes 0=>" << max << ")!" << std::endl;
	throw larbys();
      }
      
      Range_t result;

      if((int)(range.first) > max) {
        if(logger().info())
	  logger().LOG(msg::kINFO,__FUNCTION__,__LINE__)
	    << "Image lower bound (" << range.first << ") too large for plane " << plane_id << " (only goes 0=>" << max << ")" << std::endl;
	return result;
      }

      if(!range.first && !range.second) {
	if(logger().info()) 
	  logger().LOG(msg::kINFO,__FUNCTION__,__LINE__)
	    << "Zero range provided. Nothing to format... " << std::endl;
	return result;
      }
      
      const int center = ( range.first  + range.second ) / 2;

      int upper_bound, lower_bound;
      if(!_compression_factor) {
	upper_bound = center + (((int)(range.second) - center + padding) / target_width) * target_width - 1;
	lower_bound = center - ((center - (int)(range.first) + padding) / target_width) * target_width ;
	if(upper_bound < (int)(range.second)) upper_bound += target_width;
	if(lower_bound > (int)(range.first) ) lower_bound -= target_width;
      }else{
	upper_bound = center + _compression_factor * target_width / 2 - 1;
	lower_bound = center - _compression_factor * target_width / 2;
      }

      if(logger().debug())
	logger().LOG(msg::kINFO,__FUNCTION__,__LINE__)
	  << "Preliminary bounds: " << lower_bound << " => " << upper_bound << std::endl;

      // Case 1: extension do not cross hard-limit boundaries
      if(lower_bound >= 0 && upper_bound < max) {
	result.first  = lower_bound;
	result.second = upper_bound;
	if(logger().info())
	  logger().LOG(msg::kINFO,__FUNCTION__,__LINE__) 
	    << "Range [a] @ plane " << plane_id
	    << " ... Before " << range.first  << " => " << range.second
	    << " ... After " << result.first << " => " << result.second
	    << std::endl;
	return result;
      }
      
      // Case 2: touching only the max bound
      if(lower_bound >=0 && upper_bound >= max) {
	// just need to cover range min from the max-edge
	result.second = max - 1;
	if(!_compression_factor) {
	  result.first  = max - target_width * ((max - (int)(range.first) - padding) / target_width);
	  if(result.first > range.first && (int)(result.first) > target_width) result.first -= target_width;
	}
	else result.first = max - (target_width * _compression_factor);

	if(logger().info())
	  logger().LOG(msg::kINFO,__FUNCTION__,__LINE__) 
	    << "Range [b] @ plane " << plane_id
	    << " ... Before " << range.first  << " => " << range.second
	    << " ... After " << result.first << " => " << result.second
	    << std::endl;
	return result;
      }
      
      // Case3: touching only the min bound
      if(upper_bound < max && lower_bound < 0) {
	result.first  = 0;
	if(!_compression_factor) {
	  result.second = target_width * (((int)(range.second) + padding) / target_width);
	  if(result.second < range.second) result.second += target_width;
	  if((int)(result.second) >= max) result.second -= target_width;
	}
	else result.second = target_width * _compression_factor-1;
	
	if(logger().info())
	  logger().LOG(msg::kINFO,__FUNCTION__,__LINE__) 
	    << "Range [c] @ plane " << plane_id
	    << " ... Before " << range.first  << " => " << range.second
	    << " ... After " << result.first << " => " << result.second
	    << std::endl;
	return result;
      }
      
      // Case 4: touching both bounds
      if(!_compression_factor) {
	while(upper_bound >= max) upper_bound -= target_width;
	while(lower_bound <  0  ) lower_bound += target_width;
      }else{
	logger().LOG(msg::kCRITICAL,__FUNCTION__,__LINE__)
	  << "Logic error: for a fixed scale factor this error should not be raised..." << std::endl;
	throw larbys();
      }

      result.first  = lower_bound;
      result.second = upper_bound;
      
      if(logger().info())
	logger().LOG(msg::kINFO,__FUNCTION__,__LINE__) 
	  << "Range [d] @ plane " << plane_id
	  << " ... Before " << range.first  << " => " << range.second
	  << " ... After " << result.first << " => " << result.second
	  << std::endl;
      return result;
    }
    
    RangeArray_t Cropper::Format(const RangeArray_t& boundary) const
    {
      art::ServiceHandle<geo::Geometry> geom;
      if(boundary.size() != geom->Nplanes()+1) {
	logger().LOG(msg::kERROR,__FUNCTION__,__LINE__)
	  << "Size of handed boundary is not # planes (wires) + 1 (time)!" << std::endl;
	throw larbys();
      }
      
      RangeArray_t result(boundary.size());   
      
      for(size_t plane=0; plane <= geom->Nplanes(); ++plane)
	
	result[plane] = Format(boundary[plane], plane);
      
      return result;
    }
    
  }
}
