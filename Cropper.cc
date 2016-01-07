#include "Cropper.h"

#include <iostream>

#include "Geometry/Geometry.h"
#include "Utilities/TimeService.h"

#include "MCBase/MCStep.h"

namespace larcaffe {

  Cropper::Cropper() {
    minpadding_t = 100;
    minpadding_w = 10;
    targetwidth = 247;
    targetheight = 247;
    fCosmicMode = false;
  }

  Cropper::~Cropper() {}
  
  void Cropper::defineBoundingBox( const std::vector< sim::MCTrack >& mctracks, MCImage& image ) {
    
    std::cout << "[Cropper] Define Bounding Box for Image from vector<MCTrack>" << std::endl;

    art::ServiceHandle<geo::Geometry> geom;
    art::ServiceHandle<util::TimeService> ts;
    
    // get planeid tokens: for now, we get the collection plane
    const std::set< geo::PlaneID >& planes = geom->PlaneIDs();
    geo::PlaneID colplane;
    for ( std::set<geo::PlaneID>::const_iterator it_p=planes.begin(); it_p!=planes.end(); it_p++ ) {
      //std::cout << "[Cropper] a plane, id=" << (*it_p).Plane << std::endl;
      if ( (*it_p).Plane==2 )
	colplane = (*it_p);
    }
      
    int wire_max = 0;
    int wire_min = 10000;
    const double cm_per_us = 0.112; // for 300 V/cm
    double earliest_t = 1e10;
    double earliest_t_atwire = 1e10;
    double latest_t_atwire = 0;
    double trig_time_us = ts->TriggerTime();
    double trig_offset_us = ts->TriggerOffsetTPC();
    
    // loop over mc track. determine if there is something to be saved
    for (int i=0; i<(int)mctracks.size(); i++) {
      const sim::MCTrack& atrack = mctracks.at(i); // a vector of steps
      for (int s=0; s<(int)atrack.size(); s++) {
	const sim::MCStep& astep = atrack.at(s);
	double pos[3] = { astep.X(), astep.Y(), astep.Z() };
	double t_ns = astep.T();
	double deltat0_us = ts->G4ToElecTime( t_ns ); // time relative to first tick
	//if ( !fCosmicMode )
	deltat0_us -= trig_time_us + trig_offset_us;
	if ( t_ns<earliest_t )
	  earliest_t = t_ns;

	double dt_at_wire_us = deltat0_us + pos[0]/cm_per_us;
	geo::WireID wireid;
	try {
	  wireid = geom->NearestWireID( pos, colplane );
	}
	catch (cet::exception& e) {
	  std::cout << "NearestWireID threw an exception. skip this point." << std::endl;
	  continue;
	}
	if ( (int)wireid.Wire<wire_min )
	  wire_min = wireid.Wire;
	if ( (int)wireid.Wire>wire_max )
	  wire_max = wireid.Wire;

	if ( dt_at_wire_us<earliest_t_atwire  )
	  earliest_t_atwire = dt_at_wire_us;
	if ( dt_at_wire_us>latest_t_atwire )
	  latest_t_atwire = dt_at_wire_us;
	std::cout << "[Cropper] track " << i << " step " << s << ": " << pos[0] << ", " << pos[1] << ", " << pos[2] 
		  << " t=" << t_ns*1.0e-3 << " us " 
		  << " dt0=" << deltat0_us << " us"
		  << " tick(@hit)=" << ts->TPCClock().Ticks( deltat0_us )
		  << " tick(@wire)=" << ts->TPCClock().Ticks( dt_at_wire_us )
		  << "; wire=" << wireid.Wire 
		  << std::endl;
      }
      
    }//end of mctrack loop

    int tick_min = ts->TPCClock().Ticks( earliest_t_atwire )-minpadding_t;
    int tick_max = ts->TPCClock().Ticks( latest_t_atwire )  +minpadding_t;
    std::cout << "[Cropper] tick_min=" << tick_min << " tick_max=" << tick_max << std::endl;
    if ( wire_max!=0 && wire_min!=10000 && ( tick_min<9600 && tick_max>0 ) ) {
      // found some tracks with active volume energy deposits

      if ( tick_min<0 )
	tick_min = 0;
      if ( tick_max>9600 )
	tick_max = 9600;

      int tick_mean = (int)((tick_min+tick_max)/2);
      int dtick = tick_max-tick_min;
      int nheights = dtick/targetheight;
      if ( nheights*targetheight!=dtick )
	nheights+=1;
      
      int dwire = wire_max-wire_min;
      int nwidths = dwire/targetwidth;
      int wire_mean = (int)((wire_max+wire_min)/2);
      if ( nwidths*targetwidth!=dwire )
	nwidths+=1;
      
      // set bounds to be multiple of target width and heights
      int tup = (nheights*targetheight)/2;
      int tdown = tup;
      if ( (tup+tdown)!=nheights*targetheight )
	tdown += 1;
      tick_min = tick_mean-tdown;
      tick_max = tick_mean+tup;
      if ( tick_min<0 ) {
	tick_min = 0;
	tick_max = tup+tdown;
      }
      else if ( tick_max>9600 ) {
	tick_max = 9600;
	tick_min = tick_max-(tup+tdown);
      }
      dtick = tick_max-tick_min;
      
      int wup = (nwidths*targetwidth)/2;
      int wdown = wup;
      if ( (wup+wdown)!=nwidths*targetwidth )
	wdown += 1;
      wire_min = wire_mean-wdown;
      wire_max = wire_mean+wup;
      if ( wire_min<0 ) {
	wire_min = 0;
	wire_max = wup+wdown;
      }
      else if ( wire_max>=3456 ) {
	wire_max = 3455;
	wire_min = wire_max - (wup+wdown);
      }
      dwire = wire_max-wire_min;
      
      std::cout << "[Cropper] Trig time: " << trig_time_us << std::endl;
      std::cout << "[Cropper] Trig offset: " << trig_offset_us  << std::endl;
      std::cout << "[Cropper] Number of mctracks: " << mctracks.size() << std::endl;
      std::cout << "[Cropper] bounding collection wires: [" << wire_min << ", " << wire_max << "] (" << dwire/targetwidth << "R" << dwire%targetwidth << ")" << std::endl;
      std::cout << "[Cropper] bounding ticks: [" << tick_min << ", " << tick_max << "] (" << dtick/targetheight << "R" << dtick%targetheight << ")" << std::endl;
      //std::cout << "[ENTER] to continue" << std::endl;
      //std::cin.get();
            
      //MCImage image;
      image.unset = false;
      image.interactionmode = 1;
      image.nu_energy_gev = 1.0;
      image.mctrack_boundingbox[0][0] = tick_min;
      image.mctrack_boundingbox[0][1] = (int)wire_min;
      image.mctrack_boundingbox[1][0] = tick_min;
      image.mctrack_boundingbox[1][1] = (int)wire_max;
      image.mctrack_boundingbox[2][0] = tick_max;
      image.mctrack_boundingbox[2][1] = (int)wire_max;
      image.mctrack_boundingbox[3][0] = tick_max;
      image.mctrack_boundingbox[3][1] = (int)wire_min;
      return;
    }
    else {
      std::cout << "[Cropper] Searched MCTrack Vector, no bounding box could be defined. return empty." << std::endl;
      //MCImage image;
      image.unset = true;
      return;
    }
  }

  void Cropper::fillImage( MCImage& image, Image& rawadcimage ) {
    int tick_min = image.mctrack_boundingbox[0][0];
    int tick_max = image.mctrack_boundingbox[2][0];
    int wire_min = image.mctrack_boundingbox[0][1];
    int wire_max = image.mctrack_boundingbox[1][1];
    int dtick = abs( tick_max-tick_min );
    int dwire = abs( wire_max-wire_min );
    int nheights = dtick/targetheight;
    int nwidths  = dwire/targetwidth;

    if ( dtick%targetheight!=0 ) {
      std::cout << "[Cropper] UNEXPECTED CROPPED HEIGHT (remainder=" << dtick%targetheight << ")" << std::endl;
    }
    if ( dwire%targetwidth!=0 ) {
      std::cout << "[Cropper] UNEXPECTED CROPPED WIDTH (remainder=" << dwire%targetwidth << ")" << std::endl;
    }

    // set the image size
    image.precompressed_collection.setSize(dtick, dwire);
    // copy crop into new image
    for (int t=0; t<dtick; t++) {
      for (int w=0; w<dwire; w++ ) {
	double value = rawadcimage.pixel( int(tick_min+t), int(wire_min+w) );
	image.precompressed_collection.setpixel( t, w, value );
      }
    }
    
    // SumPool into target size
    std::cout << "[Cropper] Compress by factor of (" << nheights << "," << nwidths << ")" << std::endl;
    image.compressed_collection.setSize( targetheight, targetwidth );
    for (int t1=0; t1<targetheight; t1++) {
      for (int w1=0; w1<targetwidth; w1++) {
	// sum the little area
	float sum = 0.0;
	for (int nh=0; nh<nheights; nh++) {
	  for (int nw=0; nw<nwidths; nw++) {
	    sum += image.precompressed_collection.pixel( t1*nheights+nh, w1*nwidths+nw );
	  }
	}
	image.compressed_collection.setpixel( t1, w1, sum );
	// end of summing loop
      }
    }
    // done.
    return;
  }

  void Cropper::crop( const std::vector< sim::MCTrack >& mctracks, Image& rawadcimage, std::vector<MCImage>& output  ) {

    // if in neutrino mode, we set a bounding box over all particles in the interaction
    // for cosmic muon mode, we crop around muons


    if ( !fCosmicMode ) {
      MCImage img;
      defineBoundingBox( mctracks, img );
      if ( !img.unset ) {
	fillImage( img, rawadcimage );
	output.emplace_back( img );
      }
    }//end of neutrino mode
    else {
      for (int i=0; i<(int)mctracks.size(); i++) {
	const sim::MCTrack& atrack = mctracks.at(i);
	std::vector< sim::MCTrack > cosmictrack;
	cosmictrack.push_back( atrack );
	MCImage img;
	defineBoundingBox( cosmictrack, img );
	if ( !img.unset ) {
	  fillImage( img, rawadcimage );
	  output.emplace_back( img );
	}
      }
    }//end of cosmic mode
    
  }//end of crop function
  
}
