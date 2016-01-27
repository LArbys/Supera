#ifndef __SUPERA_CROPPER_H__
#define __SUPERA_CROPPER_H__

#include <vector>

// LArSoft
#include "MCBase/MCTrack.h"
#include "MCBase/MCShower.h"
#include "Simulation/SimChannel.h"

// LArCaffe
#include "LArCaffe/larcaffe_base.h"

namespace larcaffe {

  namespace supera {
    
    class Cropper : public larcaffe_base {

    public:
      
      Cropper(unsigned int time_padding=100,
	      unsigned int wire_padding=10,
	      unsigned int target_width=247,
	      unsigned int target_height=247,
	      unsigned int compression_factor=0);
      
      virtual ~Cropper() {}
      
      void configure(unsigned int time_padding,
		     unsigned int wire_padding,
		     unsigned int target_width,
		     unsigned int target_height,
		     unsigned int compression_factor);
      /**
	 Given single MCTrack, returns length 4 range array (3 planes + time) \n
	 which contains all trajectory points of input MCTrack.
      */
      RangeArray_t WireTimeBoundary( const sim::MCTrack& mct                ) const;
      RangeArray_t WireTimeBoundary( const sim::MCShower& mcsh                ) const;
      /**
	 Given a collection of MCTracks, returns length 4 range array (3 planes + time) \n
	 which contains all trajectory points of all input MCTracks.
      */
      RangeArray_t WireTimeBoundary( const std::vector<sim::MCTrack>& mct_v ) const;
      RangeArray_t WireTimeBoundary( const std::vector<sim::MCTrack>& mct_v, const std::vector<sim::MCShower>& mcsh_v ) const;
      /** 
	  Given a collection of SimChannel, returns length 4 range array (3 planes + time) \n
	  which contains all energy deposition points of input.
      */
      RangeArray_t WireTimeBoundary( const std::vector<sim::SimChannel>& simch_v) const;

      /**
	 Given a range and corresponding plane ID (plan_id == #plane is considered time), \n
	 perform padding & cropping to a multiple of target width/height.
       */
      Range_t      Format( const Range_t& range, unsigned short plane_id ) const;
      /**
	 Given a set of ranges (all planes & time), \n
	 perform padding & cropping to a multiple of target width/height.
       */
      RangeArray_t Format( const RangeArray_t& boundary                  ) const;

      unsigned int TargetWidth()  const { return _target_width;  }
      unsigned int TargetHeight() const { return _target_height; }

    protected:
      
      unsigned int _time_padding;  ///< Padding in time axis (height) for Cropper::Format function
      unsigned int _wire_padding;  ///< Padding in wire axis (width) for Cropper::Format function
      unsigned int _target_width;  ///< Unit-size (horizontal, wire, or width) for an output image of Cropper::Format function
      unsigned int _target_height; ///< Unit-size (vertical, time, or height) for an output image of Cropper::Format function
      /**
	 A scale factor used to compress image. If 0, original image is sampled in both height in \n
	 multiple of target size to contain the ROI and automatically compressed (i.e. compression \n
	 factor is computed per image and varies). If set to non-zero value, image is sampled from \n
	 the ROI center for the target size times this compression factor, then compressed (i.e. \n
	 sample image size and hence compression factor stays constant from one image to another). 
       */
      unsigned int _compression_factor;
    };
  }
}


#endif
