#include "RegionDefiner.h"

namespace larbys {
  namespace util {

    RegionDefiner::RegionDefiner( int ncolwires, int tickstart, int tickend, int tickblock, double wirepitch_cm ) {
      fNColWires = ncolwires;
      fTickStart = tickstart;
      fTickEnd   = tickend;
      fTickBlock = tickblock;
      fWirePitch_cm = wirepitch_cm;
      kCollectionPlaneIndex = 2;
      kZ0 = 0.0;
      kYTotLength_cm = 233.0;
      kY0 = -116.0;
    }

    RegionDefiner::~RegionDefiner() {
      
    }

    void RegionDefiner::configure() {
      
      art::ServiceHandle<geo::Geometry> geom; //< LArSoft

      mImgWidth_cm = fWirePitch_cm*fNColWires;

      // Time parameters
      mNumDSticks = ( fTickEnd-fTickStart )/fTickBlock;
      mNT = mNumDSticks/mNumDSticks;
      if ( mNumDSticks%mNumDSticks!=0 ) mNT++;

      // Z-direction parameters
      mTotColWires = geom->Nwires(kCollectionPlaneIndex);
      mNZ = mTotColWires/fNColWires;
      if ( mTotColWires%fNColWires!=0 ) mNZ++;
      
      // Y-Direction parameters
      mNY = (int)kYTotLength_cm/mImgWidth_cm;
      if ( mNY*mImgWidth_cm!=kYTotLength_cm ) mNY++;

    }

    void RegionDefiner::defineRegions() {
      // we do this by defining boxes in the Y-Z plane: this amounts to four line segments
      // for each box, we loop through all wires
      //  and we check if the wires intersect with any of the four segments that define the box
      // if the wire interesects, it becomes part of the region
      // the min and max wires define the region, assumed to be consecutive

      mLargestNwires = 0;
      mNregions =0;
      
      for ( int iz=0; iz<mNZ; iz++ ) {
	for (int iy=0; iy<mNY; iy++ ) {
	  
	  // define the four line segments that sets the boundary
	  // do this by 4 rays.
	  // start in lower right corner of box
	  // direction vectors move counter-clockwise

	  // define the ray origins
	  double seg0[4][2];
	  seg0[0][0] = kY0;
	  seg0[0][1] = kZ0;
	  seg0[1][0] = kY0;
	  seg0[1][1] = kZ0 + mImgWidth_cm;
	  seg0[2][0] = kY0 + mImgWidth_cm;
	  seg0[2][1] = kZ0 + mImgWidth_cm;
	  seg0[3][0] = kY0 + mImgWidth_cm;
	  seg0[3][1] = kZ0;

	  // direction vectors of the rays
	  double dir[4][2] = { {0.0, 1.0},
			       {1.0, 0.0},
			       {0.0, -1.0},
			       {-1.0,0.0} };

	  // now the fun begins, we test the intersection of wires with these line segments
	  std::vector<int> intersecting_plane_wires[3];
	  int nwires = 0;
	  for ( geo::WireID const& wID : geom->IterateWireIDs() ) {
	    geo::WireGeo const& Wire = geom->Wire( wID );
	    double start[3] = {0}; 
	    double end[3] = {0};
	    Wire.GetStart( start );
	    Wire.GetEnd( end );

	    // calculate wire direction
	    // note. the geometry information from LArSoft is in R3 with (x,y,z)
	    // we do the intersection tests in R2 (y,z)
	    double wiredir[2] = {0};
	    double wirelen = 0.0;
	    for (int v=0; v<2; v++) {
	      wiredir[v] = end[v+1]-start[v+1]; // we skip x
	      wirelen += wiredir[v]*wiredir[v];
	    }
	    wirelen = sqrt(wirelen);
	    for (int v=0; v<2; v++)
	      wiredir[v] /= wirelen;

	    // loop over each segment
	    for (unsigned int s=0; s<4; s++) {

	      // calculate useful cross products
	      double qp[2] = {0};
	      double segdir[2] = {0};
	      double rxs[3] = {0};
	      double qpxr[3] = {0};
	      double qpxs[3] = {0};
	      for (unsigned int v=0; v<2; v++) {
		qp[v] = start[v+1]-seg0[s][v];
		segdir[v] = dir[s][v];
	      }
	      rxs[2] =  cross2D( segdir, wiredir );
	      qpxr[2] = cross2D( qp, segdir);
	      qpxs[2] = cross2D( qp, wiredir);
	      
	      // test if colinear
	      bool qpxr_zero = true;
	      bool rxs_zero = true;
	      for (int v=0; v<3; v++) {
		if ( fabs(rxs[v])>1.0e-8 )
		  rxs_zero = false;
		if ( fabs(qpxr[v])>1.0e-8 )
		  qpxr_zero = false;
	      }

	      // test if non-intersecting
	      if ( rxs_zero && !qpxr_zero ) {
		//std::cout << " seg " << s << " is non-intersecting" << std::endl;
		continue;
	      }

	      if ( qpxr_zero && rxs_zero ) {
		// colinear, continue
		//std::cout << " seg " << s << " is colinear" << std::endl;
		continue;
	      }

	      // so infinite lines intersect
	      double u,t;
	      if ( rxs[2]!=0 ) {
		u = qpxr[2]/rxs[2];
		t = qpxs[2]/rxs[2];
	      }
	      else {
		u = -1;
		t = -1;
	      }
	      
	      // but do the line segements intersect?
	      if ( u>=0 && u<=wirelen && t>=0 && t<=imgwidth_cm ) {
		//std::cout << " seg " << s << " intersects" << std::endl;
		if ( fTestDraw ) {
		  TLine* l = new TLine( start[2], start[1], end[2], end[1] );
		  l->Draw();
		}
		nwires++;
		// save the intersecting wire
		intersecting_plane_wires[ wID.Plane ].push_back( wID.Wire );
	      }

	      //std::cout << " seg " << s << " neither intersects nor is parallel" << std::endl;
	    }//end of loop over box segment rays

	  }//end of loop over all wires
	

	  // OK redefine the regions.
	  // loop over planes
	  // find max and min of the region
	  // loop over time boxes
	  // make a rangearray object for this region

	  //std::cin.get();
	  
	}// loop over Y direction
      }// loop over Z direction
    }//end of defineRegions

  }//end of larbys::util
} //end of larbys
