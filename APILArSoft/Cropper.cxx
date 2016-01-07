#include "Cropper.h"

#include <iostream>

#include "Geometry/Geometry.h"
#include "Utilities/TimeService.h"
#include "Utilities/DetectorProperties.h"
#include "Utilities/LArProperties.h"
#include "Simulation/SimChannel.h"
#include "MCBase/MCTrack.h"
#include "LArCaffe/larbys.h"

namespace larcaffe {


  Cropper::Cropper(unsigned int time_padding,
		   unsigned int wire_padding,
		   unsinged int target_width,
		   unsigned int target_height)
    : _time_padding(time_padding)
    , _wire_padding(wire_padding)
    , _target_width(target_width)
    , _target_height(target_height)
  {}

  void Cropper::configure(unsigned int time_padding,
			  unsigned int wire_padding,
			  unsigned int target_width,
			  unsigned int target_height)
  {
    _time_padding = time_padding;
    _wire_padding = wire_padding;
    _target_width = target_width;
    _target_height = target_height;
  }

  RangeArray_t Cropper::WireTimeBoundary(const std::vector<sim::MCTrack>& mct_v) const
  {
    RangeArray_t result(4); // result is 3 planes' wire boundary + time boundary (4 elements)

    for(auto& r : result) { r.first = 1e12; r.second = 0; }

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
    art::ServiceHandle<util::TimeService> ts;
    const double drift_velocity = larp->DriftVelocity() * 1.e3; // make it nano-sec/cm
    double xyz[3] = {0.};

    RangeArray_t result(4); // result is 3 planes' wire boundary + time boundary (4 elements)

    for(auto& r : result) { r.first = 1e12; r.second = 0; }

    for(auto& step : mct) {

      // Figure out time
      unsigned int tick = (unsigned int)(ts->TPCG4Time2Tick(step.T() + (step.X() / larp->DriftVelocity())*1.e3)) + 1;
      
      auto& trange = result.back();
      if(trange.first  > tick) trange.first  = tick;
      if(trange.second < tick) trange.second = tick;

      // Figure out wire per plane
      xyz[0] = step.X();
      xyz[1] = step.Y();
      xyz[2] = step.Z();
      for(size_t plane=0; plane < geom->Nplanes(); ++plane) {

	unsinged int wire = geom->NearestWireID(xyz, plane);

	if(result[plane].first  > wire) result[plane].first  = wire;
	if(result[plane].second < wire) result[plane].second = wire;

      }
    }

    for(auto& r : result) 

      if(r.first == 1e12 && r.second == 0) r.first = 0;

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

	if(!time_range.first && !time_range.second)

	  time_range.first = time_range.second = tdc_ide.first;

	else {
	  if(time_range.first  > tdc_ide.first) time_range.first  = tdc_ide.first;
	  if(time_range.second < tdc_ide.first) time_range.second = tdc_ide.first;
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

    Range_t result;

    const int center = ( range.first  + range.second ) / 2;

    int upper_bound = center + (((int)(range.second) - center + padding) / target_width) * target_width - 1;
    int lower_bound = center - ((center - (int)(range.first) + padding) / target_width) * target_width ;
    if(upper_bound < range.second) upper_bound += target_width;
    if(lower_bound > range.first ) lower_bound -= target_width;

    // Case 1: extension do not cross hard-limit boundaries
    if(lower_bound >= 0 && upper_bound < max) {
      result.first  = lower_bound;
      result.second = upper_bound;
      if(logger().info())
	logger().LOG(msg::kINFO,__FUNCTION__,__LINE__) 
	  << "Plane " << plane_id 
	  << " bound " << result.first << " => " << result.second << std::endl;
      return result;
    }

    // Case 2: touching only the max bound
    if(lower_bound >=0 && upper_bound >= max) {
      // just need to cover range min from the max-edge
      result.second = max - 1;
      result.first  = max - target_width * ((max - (int)(range.first) - padding) / target_width);
      if(result.first > range.first && result.first > target_width) result.first -= target_width;
      if(logger().info())
	logger().LOG(msg::kINFO,__FUNCTION__,__LINE__) 
	  << "Plane " << plane_id 
	  << " bound " << result.first << " => " << result.second << std::endl;
      return result;
    }

    // Case3: touching only the min bound
    if(upper_bound < max && lower_bound < 0) {
      result.first  = 0;
      result.second = target_width * (((int)(range.second) + padding) / target_width);
      if(result.second < range.second) result.second += target_width;
      if(result.second >= max) result.second -= target_width;
      if(logger().info())
	logger().LOG(msg::kINFO,__FUNCTION__,__LINE__) 
	  << "Plane " << plane_id 
	  << " bound " << result.first << " => " << result.second << std::endl;
      return result;
    }

    // Case 4: touching both bounds
    while(upper_bound >= max) upper_bound -= target_width;
    while(lower_bound <  0  ) lower_bound += target_width;

    if(logger().info())
      logger().LOG(msg::kINFO,__FUNCTION__,__LINE__) 
	<< "Plane " << plane_id 
	<< " bound " << result.first << " => " << result.second << std::endl;
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
