#ifndef __LARCV_UTILS_H__
#define __LARCV_UTILS_H__

#include "LArCVTypes.h"

namespace larcv {

  bool RangeFilled(const larcv::Range_t& r);

  bool RangeEmpty( const larcv::Range_t& r );

  bool RangeValidity(const larcv::Range_t& r);

  bool RangeExclusive(const larcv::Range_t& lhs, const larcv::Range_t& rhs);

  bool RangeInclusive(const larcv::Range_t& lhs, const larcv::Range_t& rhs);

  larcv::Range_t AND(const larcv::Range_t& lhs, const larcv::Range_t& rhs);

  larcv::Range_t OR(const larcv::Range_t& lhs, const larcv::Range_t& rhs);

  bool RangesOK( const RangeArray_t& ranges );

  void PrintRangeArray( const larcv::RangeArray_t& ranges );
  
}
#endif
