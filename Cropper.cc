#include "Cropper.h"

#include <iostream>

#include "Geometry/Geometry.h"
#include "Utilities/TimeService.h"

#include "MCBase/MCStep.h"

namespace larcaffe {

  Cropper::Cropper() {
    minpadding_t = 50;
    minpadding_w = 10;
    targetwidth = 247;
    targetheight = 247;
  }

  Cropper::~Cropper() {}

  void Cropper::crop( const std::vector< sim::MCTrack >& mctracks, Image& rawadcimage, std::vector<MCImage>& output  ) {

    art::ServiceHandle<geo::Geometry> geom;
    art::ServiceHandle<util::TimeService> ts;

    const std::set< geo::PlaneID >& planes = geom->PlaneIDs();
    geo::PlaneID colplane;
    for ( std::set<geo::PlaneID>::const_iterator it_p=planes.begin(); it_p!=planes.end(); it_p++ ) {
      std::cout << "[Cropper] a plane, id=" << (*it_p).Plane << std::endl;
      if ( (*it_p).Plane==2 )
	colplane = (*it_p);
    }
      

    int wire_max = 0;
    int wire_min = 10000;
    const double cm_per_us = 0.11; // for 300 V/cm
    double earliest_t = 1e10;
    double earliest_t_atwire = 1e10;
    double latest_t_atwire = 0;
    //int zmax = 0;
    //int zmin = 100000000;
    double trig_time_us = ts->TriggerTime();



    // loop over mc track. determine if there is something to be saved
    for (int i=0; i<(int)mctracks.size(); i++) {
      const sim::MCTrack& atrack = mctracks.at(i); // a vector of steps
      for (int s=0; s<(int)atrack.size(); s++) {
	const sim::MCStep& astep = atrack.at(s);
	double pos[3] = { astep.X(), astep.Y(), astep.Z() };
	double t_ns = astep.T();
	double deltat0_us = ts->G4ToElecTime( t_ns );
	if ( t_ns<earliest_t )
	  earliest_t = t_ns;

	double dt_at_wire_us = deltat0_us + pos[0]/cm_per_us - trig_time_us;

	//double tadjust = t + (astep.X()/cm_per_s)*1e9; // ns
	//double tick = ts->TPCG4Time2TDC(  );
	//geo::WireID wireid;
	geo::WireID wireid = geom->NearestWireID( pos, colplane );  ///const double worldLoc[3], geo::PlaneID const& planeid) const;
	if ( (int)wireid.Wire<wire_min )
	  wire_min = wireid.Wire;
	if ( (int)wireid.Wire>wire_max )
	  wire_max = wireid.Wire;

	if ( dt_at_wire_us<earliest_t_atwire  )
	  earliest_t_atwire = dt_at_wire_us;
	if ( dt_at_wire_us>latest_t_atwire )
	  latest_t_atwire = dt_at_wire_us;
	std::cout << "[Cropper] track " << i << " step " << s << ": " << pos[0] << ", " << pos[1] << ", " << pos[2] 
		  << " t=" << t_ns*1.0e-3 << " uss " 
		  << " dt0=" << deltat0_us << " us"
		  << " tick(@hit)=" << ts->TPCClock( deltat0_us ).Ticks()+3200
		  << " tick(@wire)=" << ts->TPCClock( dt_at_wire_us ).Ticks()+3200
		  << "; wire=" << wireid.Wire 
		  << std::endl;
      }
      
    }//end of mctrack loop


    if ( wire_max!=0 && wire_min!=10000 ) {
      // found some tracks with active volume energy deposits
      int tick_min = ts->TPCClock( earliest_t_atwire ).Ticks()+3200-minpadding_t;
      int tick_max = ts->TPCClock( latest_t_atwire ).Ticks()+3200+minpadding_t;
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
      if ( (tup+tdown)!=targetheight )
	tdown += 1;
      tick_min = tick_mean-tdown;
      tick_max = tick_mean+tup;
      if ( tick_min<0 ) {
	tick_min = 0;
	tick_max = tup+tdown;
      }
      else if ( tick_max>=9600 ) {
	tick_max = 9599;
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
	wire_min = wire_max = (wup+wdown);
      }
      dwire = wire_max-wire_min;
      
      std::cout << "[Cropper] Trig time: " << trig_time_us << std::endl;
      std::cout << "[Cropper] Number of mctracks: " << mctracks.size() << std::endl;
      std::cout << "[Cropper] bounding collection wires: [" << wire_min << ", " << wire_max << "]" << std::endl;
      std::cout << "[Cropper] bounding ticks: [" << tick_min << ", " << tick_max << "]" << std::endl;
      std::cout << "[ENTER] to continue" << std::endl;
      std::cin.get();
            
      MCImage image;
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
      image.precompressed_collection.setSize(dtick, dwire);
      
      // copy crop into new image
      for (int t=0; t<dtick; t++) {
	for (int w=0; w<dwire; w++ ) {
	  double value = rawadcimage.pixel( int(tick_min+t), int(wire_min+w) );
	  image.precompressed_collection.setpixel( t, w, value );
	}
      }
      
      output.emplace_back( image );
    }// if crop found
    else {
      std::cout << "[Cropper] No charged tracks through active voluem." << std::endl;
      std::cout << "[ENTER] to continue." << std::endl;
      std::cin.get();
    }
  }
  
}
