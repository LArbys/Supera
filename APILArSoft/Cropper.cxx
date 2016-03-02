#include <iostream>

#include "Cropper.h"

#include "Geometry/Geometry.h"
#include "Utilities/TimeService.h"
#include "Utilities/DetectorProperties.h"
#include "Utilities/LArProperties.h"
#include "MCBase/MCStep.h"

#include "LArCaffe/larbys.h"
#include "LArCaffe/LArCaffeUtils.h"

#include "TLorentzVector.h" // ROOT

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
      
      for(auto& r : result) { r.start = larcaffe::kINVALID_UINT; r.end = 0; }

      for(auto const& mct : mct_v) {
	
	auto boundary = WireTimeBoundary(mct);

	// check if empty
	
	// if all are [0,0], useless
	bool allzero = true;
	for ( size_t plane=0; plane<boundary.size(); plane++ ) {
	  if ( boundary[plane].start!=0 || boundary[plane].end!=0 ) {
	    allzero = false;
	    break;
	  }
	}
	// check if never updated
	bool isempty = true;
	for ( size_t plane=0; plane<boundary.size(); plane++ ) {
	  if ( boundary[plane].start!=larcaffe::kINVALID_UINT || boundary[plane].end!=0 ) {
	    isempty = false;
	    break;
	  }
	}
	
	if ( isempty || allzero )
	  continue;
	
	for(size_t i=0; i<result.size(); ++i) {
	  
	  auto& all = result[i];
	  auto& one = boundary[i];
	  if(all.start  > one.start ) all.start  = one.start;
	  if(all.end < one.end) all.end = one.end;
	  
	}
      }
      
      if(logger().info()) {
	
	for(size_t plane=0; plane <= geom->Nplanes(); ++plane)
	  
	  logger().LOG(msg::kINFO,__FUNCTION__,__LINE__) 
	    << "MCTrack Collection ... Plane " << plane 
	    << " bound " << result[plane].start << " => " << result[plane].end << std::endl;
      }
      
      return result;
    }
    
    RangeArray_t Cropper::WireTimeBoundary(const std::vector<sim::MCTrack>& mct_v, const std::vector<sim::MCShower>& mcsh_v ) const
    {
      art::ServiceHandle<geo::Geometry> geom;

      RangeArray_t result(4); // result is 3 planes' wire boundary + time boundary (4 elements)
      
      for(auto& r : result) { r.start = larcaffe::kINVALID_UINT; r.end = 0; }
    
      // MCTRACKS
      for(auto const& mct : mct_v) {
	
	auto boundary = WireTimeBoundary(mct);

	// check if empty
	
	// if all are [0,0], useless
	bool allzero = true;
	for ( size_t plane=0; plane<boundary.size(); plane++ ) {
	  if ( boundary[plane].start!=0 || boundary[plane].end!=0 ) {
	    allzero = false;
	    break;
	  }
	}
	// check if never updated
	bool isempty = true;
	for ( size_t plane=0; plane<boundary.size(); plane++ ) {
	  if ( boundary[plane].start!=larcaffe::kINVALID_UINT || boundary[plane].end!=0 ) {
	    isempty = false;
	    break;
	  }
	}
	
	if ( isempty || allzero )
	  continue;
	
	for(size_t i=0; i<result.size(); ++i) {
	  
	  auto& all = result[i];
	  auto& one = boundary[i];
	  if(all.start  > one.start ) all.start  = one.start;
	  if(all.end < one.end) all.end = one.end;
	  
	}
      }

      // MCSHOWERS
      for(auto const& mcsh : mcsh_v) {
	
	auto boundary = WireTimeBoundary(mcsh);

	// check if empty
	
	// if all are [0,0], useless
	bool allzero = true;
	for ( size_t plane=0; plane<boundary.size(); plane++ ) {
	  if ( boundary[plane].start!=0 || boundary[plane].end!=0 ) {
	    allzero = false;
	    break;
	  }
	}
	// check if never updated
	bool isempty = true;
	for ( size_t plane=0; plane<boundary.size(); plane++ ) {
	  if ( boundary[plane].start!=larcaffe::kINVALID_UINT || boundary[plane].end!=0 ) {
	    isempty = false;
	    break;
	  }
	}
	
	if ( isempty || allzero )
	  continue;
	
	for(size_t i=0; i<result.size(); ++i) {
	  
	  auto& all = result[i];
	  auto& one = boundary[i];
	  if(all.start  > one.start ) all.start  = one.start;
	  if(all.end < one.end) all.end = one.end;
	  
	}
      }
      
      if(logger().info()) {
	
	for(size_t plane=0; plane <= geom->Nplanes(); ++plane)
	  
	  logger().LOG(msg::kINFO,__FUNCTION__,__LINE__) 
	    << "MCTrack Collection ... Plane " << plane 
	    << " bound " << result[plane].start << " => " << result[plane].end << std::endl;
      }
      
      return result;
    }
    
    RangeArray_t Cropper::WireTimeBoundary(const ::sim::MCTrack& mct) const
    {
      art::ServiceHandle<geo::Geometry> geom;
      art::ServiceHandle<util::LArProperties> larp;
      art::ServiceHandle<util::DetectorProperties> detp;
      art::ServiceHandle<util::TimeService> ts;
      const double drift_velocity = larp->DriftVelocity()*1.0e-3; // make it cm/ns
      const int tick_max = detp->NumberTimeSamples();
      const double wireplaneoffset_cm = 7.0; //cm (made up)
      double xyz[3] = {0.};
      
      RangeArray_t result(4); // result is 3 planes' wire boundary + time boundary (4 elements)
      
      for(auto& r : result) { r.start = larcaffe::kINVALID_UINT; r.end = 0; }
      
      for(auto& step : mct) {
	
	// Figure out time
	int tick = (unsigned int)(ts->TPCG4Time2Tick(step.T() + ((step.X()+wireplaneoffset_cm) / drift_velocity))) + 1;

	if(tick < 0 || tick >= tick_max) continue;
	
	auto& trange = result.back();
	if(trange.start  > (unsigned int)tick) trange.start  = tick;
	if(trange.end < (unsigned int)tick) trange.end = tick;
	
	// Figure out wire per plane
	xyz[0] = step.X();
	xyz[1] = step.Y();
	xyz[2] = step.Z();
	for(size_t plane=0; plane < geom->Nplanes(); ++plane) {

	  geo::WireID wire_id;
	  try {
	    wire_id = geom->NearestWireID(xyz, plane);
	  }
	  catch (geo::InvalidWireIDError& err) {
	    //std::cout << "out of bounds. using better number" << std::endl;
	    // if ( std::fabs(xyz[2]-1000.0) < std::fabs(xyz[2]) )
	    // 	wire_id.Wire = geom->Nwires(plane);
	    // else
	    // 	wire_id.Wire = 0;
	    wire_id.Wire = err.better_wire_number;
	  }
	  catch (...) {
	    //std::cout << "out of bounds. using better number" << std::endl;
	    if ( std::fabs(xyz[2]-1000.0) < std::fabs(xyz[2]) )
	      	wire_id.Wire = geom->Nwires(plane);
	    else
	      wire_id.Wire = 0;
	  }
	  
	  
	  if(result[plane].start  > wire_id.Wire) result[plane].start  = wire_id.Wire;
	  if(result[plane].end < wire_id.Wire) result[plane].end = wire_id.Wire;
	  result[plane].setFilled();
	}
	result.back().setFilled();
	if(logger().info() ) {
	  logger().LOG(msg::kINFO,__FUNCTION__,__LINE__)
	    << "x=" << xyz[0]
	    <<" : t="<<step.T() << " ns"
	    <<" v=" << drift_velocity 
	    << " t+x/v=" << step.T() + (step.X() / drift_velocity) 
	    << " tick=" << tick
	    << " ... "
	      << " z=" << xyz[2]
	    //<< " wire=" << geom->NearestWireID(xyz, 2) 
	    << " result[plane2]=[" << result[2].start << ", " << result[2].end << "]"
	    << std::endl;
	}
	
      }
      
      for(auto& r : result) 
	
	if(r.start == larcaffe::kINVALID_UINT && r.end == 0) r.start = 0;
      
      if(logger().info()) {
	
	for(size_t plane=0; plane <= geom->Nplanes(); ++plane)
	  
	  logger().LOG(msg::kINFO,__FUNCTION__,__LINE__) 
	    << "Single MCTrack ... Plane " << plane 
	    << " bound " << result[plane].start << " => " << result[plane].end << std::endl;
      }
      
      return result;
    }

    RangeArray_t Cropper::WireTimeBoundary(const ::sim::MCShower& mcsh) const
    {
      art::ServiceHandle<geo::Geometry> geom;
      art::ServiceHandle<util::LArProperties> larp;
      art::ServiceHandle<util::DetectorProperties> detp;
      art::ServiceHandle<util::TimeService> ts;
      const double drift_velocity = larp->DriftVelocity()*1.0e-3; // make it cm/ns
      const int tick_max = detp->NumberTimeSamples();
      double xyz[3] = {0.};
      
      RangeArray_t result(4); // result is 3 planes' wire boundary + time boundary (4 elements)
      
      for(auto& r : result) { r.start = larcaffe::kINVALID_UINT; r.end = 0; }

      const sim::MCStep& detprofile = mcsh.DetProfile();
      double energy = detprofile.E();
      if (energy<1 )
	return result;
      double showerlength = 13.8874 + 0.121734*energy - (3.75571e-05)*energy*energy;
      showerlength *= 2.5;
      //double showerlength = 100.0;
      double detprofnorm = sqrt( detprofile.Px()*detprofile.Px() + detprofile.Py()*detprofile.Py() + detprofile.Pz()*detprofile.Pz() );
      TLorentzVector showerend;
      const double wireplaneoffset_cm = 7.0; //cm (made up)
      showerend[0] = detprofile.X()+showerlength*(detprofile.Px()/detprofnorm); 
      showerend[1] = detprofile.Y()+showerlength*(detprofile.Py()/detprofnorm); 
      showerend[2] = detprofile.Z()+showerlength*(detprofile.Pz()/detprofnorm); 
      showerend[3] = detprofile.T();
      //std::cout << "showerlength: " << showerlength << " norm=" << detprofnorm << std::endl;
      
      std::vector< TLorentzVector > step_v;
      step_v.push_back( detprofile.Position() );
      step_v.push_back( showerend );
      
      for(auto& step : step_v) {
	
	// Figure out time
	int tick = (unsigned int)(ts->TPCG4Time2Tick(step.T() + ( (step.X()+wireplaneoffset_cm) / drift_velocity))) + 1;

	if(tick < 0 || tick >= tick_max) continue;
	
	auto& trange = result.back();
	if(trange.start  > (unsigned int)tick) trange.start  = tick;
	if(trange.end < (unsigned int)tick) trange.end = tick;
	
	// Figure out wire per plane
	xyz[0] = step.X();
	xyz[1] = step.Y();
	xyz[2] = step.Z();
	for(size_t plane=0; plane < geom->Nplanes(); ++plane) {

	  try{
	    geo::WireID wire_id;
	    try {
	      wire_id = geom->NearestWireID(xyz, plane);
	    }
	    catch (geo::InvalidWireIDError& err) {
	      //std::cout << "out of bounds. using better number" << std::endl;
	      // if ( std::fabs(xyz[2]-1000.0) < std::fabs(xyz[2]) )
	      // 	wire_id.Wire = geom->Nwires(plane);
	      // else
	      // 	wire_id.Wire = 0;
	      wire_id.Wire = err.better_wire_number;
	    }
	    catch (...) {
	      //std::cout << "out of bounds. using better number" << std::endl;
	      if ( std::fabs(xyz[2]-1000.0) < std::fabs(xyz[2]) )
	      	wire_id.Wire = geom->Nwires(plane);
	      else
	      	wire_id.Wire = 0;
	    }
	  
	    if(result[plane].start  > wire_id.Wire) result[plane].start  = wire_id.Wire;
	    if(result[plane].end < wire_id.Wire) result[plane].end = wire_id.Wire;
	    result[plane].setFilled();
	  }
	  catch(...) {continue;}

	}
	result.back().setFilled();

	if(logger().info() ) {
	  logger().LOG(msg::kINFO,__FUNCTION__,__LINE__)
	    << "MCSHOWER x=" << xyz[0]
	    <<" : t="<<step.T() << " ns"
	    <<" v=" << drift_velocity 
	    << " t+x/v=" << step.T() + (step.X() / drift_velocity) 
	    << " tick=" << tick
	    << " ... "
	    << " z=" << xyz[2]
	    //<< " wire=" << wire_id.Wire
	    << " result[plane2]=[" << result[2].start << ", " << result[2].end << "]"
	    << std::endl;
	}

      }
      
      for(auto& r : result) 
	
	if(r.start == larcaffe::kINVALID_UINT && r.end == 0) r.start = 0;
      
      if(logger().info()) {
	
	for(size_t plane=0; plane <= geom->Nplanes(); ++plane)
	  
	  logger().LOG(msg::kINFO,__FUNCTION__,__LINE__) 
	    << "Single MCShower ... Plane " << plane 
	    << " bound " << result[plane].start << " => " << result[plane].end << std::endl;
      }
      
      return result;
    }

    RangeArray_t Cropper::WireTimeBoundary(const ::simb::MCParticle& mct) const
    {
      art::ServiceHandle<geo::Geometry> geom;
      art::ServiceHandle<util::LArProperties> larp;
      art::ServiceHandle<util::DetectorProperties> detp;
      art::ServiceHandle<util::TimeService> ts;
      const double drift_velocity = larp->DriftVelocity()*1.0e-3; // make it cm/ns
      const int tick_max = detp->NumberTimeSamples();
      double xyz[3] = {0.};
      
      RangeArray_t result(4); // result is 3 planes' wire boundary + time boundary (4 elements)
      
      for ( auto &traj : mct.Trajectory() ) {
	
	// Figure out time
	int tick = (unsigned int)(ts->TPCG4Time2Tick( traj.first.T() + ( traj.first.X() / drift_velocity))) + 1;

	if(tick < 0 || tick >= tick_max) {
	  std::cout << "out of time tick range: " << tick << std::endl;
	  continue;
	}
	
	auto& trange = result.back();
	if(trange.start  > (unsigned int)tick) trange.start  = tick;
	if(trange.end < (unsigned int)tick) trange.end = tick;
	
	// Figure out wire per plane
	xyz[0] = traj.first.X();
	xyz[1] = traj.first.Y();
	xyz[2] = traj.first.Z();
	// xyz[0] = mct.Vx(istep);
	// xyz[1] = mct.Vy(istep);
	// xyz[2] = mct.Vz(istep);
	for(size_t plane=0; plane < geom->Nplanes(); ++plane) {

	  geo::WireID wire_id;
	  try {
	    wire_id = geom->NearestWireID(xyz, plane);
	  }
	  catch (geo::InvalidWireIDError& err) {
	    std::cout << "Invalid Wire (x,y,z)=(" << xyz[0] << ", " << xyz[1] << ", " << xyz[2] << ")" << std::endl;
	    wire_id.Wire = err.better_wire_number;
	  }
	  catch (...) {
	    //std::cout << "out of bounds. using better number" << std::endl;
	    if ( std::fabs(xyz[2]-1000.0) < std::fabs(xyz[2]) )
	      	wire_id.Wire = geom->Nwires(plane);
	    else
	      wire_id.Wire = 0;
	  }
	  
	  if(result[plane].start  > wire_id.Wire) result[plane].start  = wire_id.Wire;
	  if(result[plane].end < wire_id.Wire) result[plane].end = wire_id.Wire;

	}
	if(logger().info() ) {
	  logger().LOG(msg::kINFO,__FUNCTION__,__LINE__)
	    << "x=" << xyz[0]
	    <<" : t="<<traj.first.T() << " ns"
	    <<" v=" << drift_velocity 
	    << " t+x/v=" << traj.first.T() + ( traj.first.X() / drift_velocity) 
	    << " tick=" << tick
	    << " ... "
	    << " z=" << xyz[2]
	    << " result[plane2]=[" << result[2].start << ", " << result[2].end << "]"
	    << std::endl;
	}
	
      }
      
      if(logger().info()) {
	
	for(size_t plane=0; plane <= geom->Nplanes(); ++plane)
	  
	  logger().LOG(msg::kINFO,__FUNCTION__,__LINE__) 
	    << "Single MCParticle ... Plane " << plane 
	    << " bound " << result[plane].start << " => " << result[plane].end << std::endl;
      }
      
      return result;
    }
    
    RangeArray_t Cropper::WireTimeBoundary( const std::vector<sim::SimChannel>& simch_v) const
    {
      
      art::ServiceHandle<geo::Geometry> geom;
      art::ServiceHandle<util::TimeService> ts;
      art::ServiceHandle<util::DetectorProperties> detp;
      RangeArray_t result(geom->Nplanes()+1);
      for(auto& range : result) { range.start = 0; range.end = 0; }
      
      for(auto const& simch : simch_v) {
	
	auto const ch = simch.Channel();
	
	auto const wid = geom->ChannelToWire(ch).front();
	
	auto& wire_range = result[wid.Plane];
	auto& time_range = result.back();
	
	if(!wire_range.start && !wire_range.end)
	  
	  wire_range.start = wire_range.end = wid.Wire;
	
	else {
	  if(wire_range.start  > wid.Wire) wire_range.start  = wid.Wire;
	  if(wire_range.end < wid.Wire) wire_range.end = wid.Wire;
	}
	
	for(auto const& tdc_ide : simch.TDCIDEMap()) {

	  auto const& tdc = tdc_ide.first;
	  
	  auto const tick = ts->TPCTDC2Tick(tdc);

	  if(tick<0 || tick >= detp->NumberTimeSamples()) continue;
	  
	  if(!time_range.start && !time_range.end)
	    
	    time_range.start = time_range.end = tick;
	  
	  else {
	    if(time_range.start  > tick) time_range.start  = tick;
	    if(time_range.end < tick) time_range.end = tick;
	  }
	}
      }
      
      if(logger().info()) {
	
	for(size_t plane=0; plane <= geom->Nplanes(); ++plane)
	  
	  logger().LOG(msg::kINFO,__FUNCTION__,__LINE__) 
	    << "Plane " << plane 
	    << " bound " << result[plane].start << " => " << result[plane].end << std::endl;
      }

      return result;
    }
    
    Range_t Cropper::Format(const Range_t& range, unsigned short plane_id) const
    {
      art::ServiceHandle<util::DetectorProperties> detp;
      art::ServiceHandle<geo::Geometry> geom;

      // we check the quality of Range_t
      // if empty we return empty range
      // if inconsistent we throw

      // never filled
      if ( larcaffe::RangeEmpty(range)  ) {
	//std::cout << "Empty Range" << std::endl;
	Range_t emptyrange(0,0);
	return emptyrange;
      }
      
      if ( !larcaffe::RangeValidity(range) ) {
	//std::cout << "Invalid Range" << std::endl;
	Range_t emptyrange(0,0);
	return emptyrange;
      }

      if(range.end < range.start) {
	logger().LOG(msg::kERROR,__FUNCTION__,__LINE__)
	  << "Inconsistent boundary given: max (" << range.end << ") < min (" << range.start << ") !" << std::endl;
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

      if((int)(range.start) > max) {
        if(logger().info())
	  logger().LOG(msg::kINFO,__FUNCTION__,__LINE__)
	    << "Image lower bound (" << range.start << ") too large for plane " << plane_id << " (only goes 0=>" << max << ")" << std::endl;
	return result;
      }
      
      // checks are over
      result.setFilled();

      if(logger().debug())
	logger().LOG(msg::kINFO,__FUNCTION__,__LINE__)
          << "Set bounds: target " << target_width  << std::endl;

      const int center = ( range.start  + range.end ) / 2;

      int upper_bound, lower_bound;
      if(!_compression_factor) {
	upper_bound = center + (((int)(range.end) - center + padding) / target_width) * target_width - 1;
	lower_bound = center - ((center - (int)(range.start) - padding) / target_width) * target_width ;
	if(upper_bound < (int)(range.end)) upper_bound += target_width;
	if(lower_bound > (int)(range.start) ) lower_bound -= target_width;
      }else{
	upper_bound = center + _compression_factor * target_width / 2 - 1;
	lower_bound = center - _compression_factor * target_width / 2;
      }

      if(logger().debug())
	logger().LOG(msg::kINFO,__FUNCTION__,__LINE__)
	  << "Preliminary bounds: " << lower_bound << " => " << upper_bound << std::endl;

      // Case 1: extension do not cross hard-limit boundaries
      if(lower_bound >= 0 && upper_bound < max) {
	result.start  = lower_bound;
	result.end = upper_bound;
	if(logger().info())
	  logger().LOG(msg::kINFO,__FUNCTION__,__LINE__) 
	    << "Range [a] @ plane " << plane_id
	    << " ... Before " << range.start  << " => " << range.end
	    << " ... After " << result.start << " => " << result.end
	    << std::endl;
	return result;
      }
      
      // Case 2: touching only the max bound
      if(lower_bound >=0 && upper_bound >= max) {
	// just need to cover range min from the max-edge
	result.end = max - 1;
	if(!_compression_factor) {
	  result.start  = max - target_width * ((max - (int)(range.start) - padding) / target_width);
	  if(result.start > range.start && (int)(result.start) > target_width) result.start -= target_width;
	}
	else result.start = max - (target_width * _compression_factor);

	if(logger().info())
	  logger().LOG(msg::kINFO,__FUNCTION__,__LINE__) 
	    << "Range [b] @ plane " << plane_id
	    << " ... Before " << range.start  << " => " << range.end
	    << " ... After " << result.start << " => " << result.end
	    << std::endl;
	return result;
      }
      
      // Case3: touching only the min bound
      if(upper_bound < max && lower_bound < 0) {
	result.start  = 0; // set to lower bound
	if(!_compression_factor) {
	  result.end = target_width * (((int)(range.end) + padding) / target_width);
	  if((result.end < range.end) || range.end==0 ) result.end += target_width;
	  if((int)(result.end) >= max) result.end -= target_width;
	  result.end -= 1;
	}
	else result.end = target_width * _compression_factor - 1;
	
	if(logger().info())
	  logger().LOG(msg::kINFO,__FUNCTION__,__LINE__) 
	    << "Range [c] @ plane " << plane_id
	    << " ... Before " << range.start  << " => " << range.end
	    << " ... After " << result.start << " => " << result.end
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

      result.start  = lower_bound;
      result.end = upper_bound;
      
      if(logger().info())
	logger().LOG(msg::kINFO,__FUNCTION__,__LINE__) 
	  << "Range [d] @ plane " << plane_id
	  << " ... Before " << range.start  << " => " << range.end
	  << " ... After " << result.start << " => " << result.end
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
   
    RangeArray_t Cropper::WireTimeBoundary( const std::vector<::larbys::supera::MCPTInfo>& interaction_bundles ) const {
      art::ServiceHandle<geo::Geometry> geom;
      RangeArray_t result(4); // result is 3 planes' wire boundary + time boundary (4 elements)
      for (auto const& mcinfo : interaction_bundles ) {
	RangeArray_t particlebounds;

	// be rid of neutral particles
	if ( mcinfo.getPDG()==2112 || mcinfo.getPDG()==22 || (mcinfo.getPDG()>10000) )
	  continue;
	
	// determine energy lost
	
	
	if ( mcinfo.isTrack() ) 
	  particlebounds = WireTimeBoundary( *(mcinfo.thetrack) );
	else if ( mcinfo.isShower() ) 
	  particlebounds = WireTimeBoundary( *(mcinfo.theshower) );
	else if ( mcinfo.isMCParticle() )
	  particlebounds = WireTimeBoundary( *(mcinfo.theparticle) );
	else
	  assert(false);

	//std::cout << "individual track/shower: ";
	larcaffe::PrintRangeArray( particlebounds );

	if ( !larcaffe::RangesOK( particlebounds ) ) {
	  //std::cout << "Range rejected for use" << std::endl;
	  continue;
	}

	for(size_t i=0; i<result.size(); ++i) {
          auto& all = result[i];
          auto& one = particlebounds[i];
          if(all.start  > one.start ) all.start  = one.start;
          if(all.end < one.end) all.end = one.end;
	}
      }//loop over interaction bundles
      
      return result;
    }
    
  }
}
