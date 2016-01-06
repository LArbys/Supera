#include "Cropper.h"

#include <iostream>

#include "Geometry/Geometry.h"
#include "Utilities/TimeService.h"

#include "MCBase/MCStep.h"

namespace larcaffe {

  Cropper::Cropper() {}

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
    //const double cm_per_s = 96153.8;
    double earliest_t = 1e10;
    //int zmax = 0;
    //int zmin = 100000000;

    // loop over mc track. determine if there is something to be saved
    for (int i=0; i<(int)mctracks.size(); i++) {
      const sim::MCTrack& atrack = mctracks.at(i); // a vector of steps
      for (int s=0; s<(int)atrack.size(); s++) {
	const sim::MCStep& astep = atrack.at(s);
	double pos[3] = { astep.X(), astep.Y(), astep.Z() };
	double t = astep.T();
	if ( t<earliest_t )
	  earliest_t = t;
	//double tadjust = t + (astep.X()/cm_per_s)*1e9; // ns
	//double tick = ts->TPCG4Time2TDC(  );
	//geo::WireID wireid;
	geo::WireID wireid = geom->NearestWireID( pos, colplane );  ///const double worldLoc[3], geo::PlaneID const& planeid) const;
	if ( (int)wireid.Wire<wire_min )
	  wire_min = wireid.Wire;
	if ( (int)wireid.Wire>wire_max )
	  wire_max = wireid.Wire;
	std::cout << "[Cropper] track " << i << " step " << s << ": " << pos[0] << ", " << pos[1] << ", " << pos[2] 
		  << "t=" << t << "; wire=" << wireid.Wire << " tick=" << ts->TPCG4Time2TDC( t ) << std::endl;
      }
      
    }//end of mctrack loop

    std::cout << "[Cropper] Number of mctracks: " << mctracks.size() << std::endl;
    std::cout << "[Cropper] bounding collection wires: [" << wire_min << ", " << wire_max << "]" << std::endl;
    std::cout << "[Cropper] bounding ticks: " << ts->TPCG4Time2TDC( earliest_t ) << " " << ts->TPCG4Time2TDC( earliest_t )+4000 << std::endl;


    if ( wire_max!=0 && wire_min!=10000 ) {
      
      if ( (wire_max-wire_min)<247 ) {
	int wire_mean = (int)((wire_max+wire_min)/2);
	wire_max = wire_mean+124;
	wire_min = wire_mean-123;
	std::cout << "[Cropper] extend cropping region: " << wire_min << ", " << wire_max << " mean=" << wire_mean << std::endl;
	if ( wire_min<0 ) {
	  wire_max += -wire_min;
	  wire_min = 0;
	}
	else if ( wire_max>=3456 ) {
	  wire_min -= wire_max-3455;
	  wire_max = 3455;
	}
      }
      
      MCImage image;
      image.interactionmode = 1;
      image.nu_energy_gev = 1.0;
      image.mctrack_boundingbox[0][0] = 3200;
      image.mctrack_boundingbox[0][1] = (int)wire_min;
      image.mctrack_boundingbox[1][0] = 3200;
      image.mctrack_boundingbox[1][1] = (int)wire_max;
      image.mctrack_boundingbox[2][0] = 3200+4800;
      image.mctrack_boundingbox[2][1] = (int)wire_max;
      image.mctrack_boundingbox[3][0] = 3200+4800;
      image.mctrack_boundingbox[3][1] = (int)wire_min;
      image.precompressed_collection.setSize(4800, (int)(wire_max-wire_min) );
      
      // copy crop into new image
      for (int t=0; t<4800; t++) {
	for (int w=0; w<int(wire_max-wire_min); w++ ) {
	  double value = rawadcimage.pixel( int(3200+t), int(wire_min+w) );
	  image.precompressed_collection.setpixel( t, w, value );
	}
      }
      
      output.emplace_back( image );
    }// if crop found
  }
  
}
