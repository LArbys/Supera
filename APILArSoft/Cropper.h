#ifndef __CROPPER__
#define __CROPPER__

#include <vector>

// LArSoft
#include "MCBase/MCTrack.h"

// LArCaffe
#include "LArCaffe/larcaffe_base.h"

namespace larcaffe {

  namespace supera {

    class Cropper : public larcaffe_base {

    public:
      
      Cropper(unsigned int time_padding=100,
	      unsigned int wire_padding=10,
	      unsigned int target_width=247,
	      unsigned int target_height=247);
      
      virtual ~Cropper() {}
      
      void configure(unsigned int time_padding,
		     unsigned int wire_padding,
		     unsigned int target_width,
		     unsigned int target_height);

      /**
	 Given single MCTrack, returns length 4 range array (3 planes + time) \n
	 which contains all trajectory points of input MCTrack.
      */
      RangeArray_t WireTimeBoundary( const sim::MCTrack& mct                ) const;
      /**
	 Given a collection of MCTracks, returns length 4 range array (3 planes + time) \n
	 which contains all trajectory points of all input MCTracks.
      */
      RangeArray_t WireTimeBoundary( const std::vector<sim::MCTrack>& mct_v ) const;
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

    protected:

      unsinged int _time_padding;  ///< Padding in time axis (height) for Cropper::Format function
      unsigned int _wire_padding;  ///< Padding in wire axis (width) for Cropper::Format function
      unsigned int _target_width;  ///< Unit-size (horizontal, wire, or width) for an output image of Cropper::Format function
      unsigned int _target_height; ///< Unit-size (vertical, time, or height) for an output image of Cropper::Format function

  };

}


#endif
