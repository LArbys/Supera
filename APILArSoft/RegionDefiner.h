#ifndef __REGIONDEFINER__
#define __REGIONDEFINER__

// This class uses the geometry service to define image regions in 
// Y-Z (microboone coordinate system)

namespace larbys {
  namespace util {

    class RegionDefiner {

    public:

      RegionDefiner( int ncolwires, int tickstart, int tickend, int tickblock, double wirepitch );
      ~RegionDefiner() {};

      void drawAndSaveImages();
      
      
      int imagewidth; // we make square regions
      std::vector< RangeArray_t > regions; // limits of regions

    protected:
      
      // user parameters
      int fNColWires;  // Number of collection wires. This defines the size of the regions in Y
      int fTickStart;  // Starting Tick of the TPC data to divide up
      int fTickEnd;    // Ending Tick of the TPC data to divide up
      int fTickBlock;  // How many Ticks to sum together
      
      void configure(); // we take the above user set parameters and determine our internal parameters


    };

  }
}

#endif
