
#include "LArCVUtils.h"
#include "larbys.h"

namespace larcv {

  bool RangeFilled(const larcv::Range_t& r)
  {
    if ( r.start==kINVALID_UINT && r.end==0 ) return false;
    return r.isFilled();
  }
  
  bool RangeEmpty( const larcv::Range_t& r ) {
    if ( r.start==0 && r.end==0 )return true;
    return false;
  }

  bool RangeValidity(const larcv::Range_t& r)
  { return (r.start <= r.end); }

  bool RangeExclusive(const larcv::Range_t& lhs, const larcv::Range_t& rhs)
  { return (lhs.end < rhs.start || rhs.end < lhs.start); }

  bool RangeInclusive(const larcv::Range_t& lhs, const larcv::Range_t& rhs)
  { return (lhs.start < rhs.start && rhs.end < lhs.end); }

  larcv::Range_t AND(const larcv::Range_t& lhs, const larcv::Range_t& rhs)
  {
    if(!RangeValidity(lhs) || !RangeValidity(rhs)) throw larbys("<larcv::AND> Invalid range provided!");

    larcv::Range_t res(0,0);

    if(RangeExclusive(lhs,rhs)) return res;

    if(RangeInclusive(lhs,rhs)) res = lhs;
    else if(RangeInclusive(rhs,lhs)) res = rhs;
    else {
      res.start = std::min(lhs.start,rhs.start);
      res.end = std::max(lhs.end,rhs.end);
    }
    return res;
  }

  larcv::Range_t OR(const larcv::Range_t& lhs, const larcv::Range_t& rhs)
  {
    if(!RangeValidity(lhs) || !RangeValidity(rhs)) throw larbys("<larcv::AND> Invalid range provided!");

    larcv::Range_t res(0,0);
    res.start = std::min(lhs.start,rhs.start);
    res.end = std::max(lhs.end,rhs.end);
    return res;
  }

  bool RangesOK( const RangeArray_t& ranges ) {
    for (int i=0; i<(int)ranges.size(); i++) {
      if ( !RangeValidity(ranges.at(i)) || RangeEmpty(ranges.at(i)) || !RangeFilled(ranges.at(i)) ) {
	return false;
      }
    }
    return true;
  }

  void PrintRangeArray( const RangeArray_t& ranges ) {
    std::cout << "RangeArray: ";
    for (int i=0; i<(int)ranges.size(); i++) {
      std::cout << " (" << i << ") ";
      
      std::cout << " [" << ranges.at(i).start << "," << ranges.at(i).end << "]";
      if (ranges.at(i).isFilled() )
	std::cout << ":filled";
      else
	std::cout << ":unfilled";
    }
    std::cout << std::endl;
  }
  
}
