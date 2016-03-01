#ifndef __LARCAFFE_UTILS_H__
#define __LARCAFFE_UTILS_H__

#include "LArCaffe/LArCaffeTypes.h"

namespace larcaffe {

  bool RangeFilled(const larcaffe::Range_t& r);

  bool RangeEmpty( const larcaffe::Range_t& r );

  bool RangeValidity(const larcaffe::Range_t& r);

  bool RangeExclusive(const larcaffe::Range_t& lhs, const larcaffe::Range_t& rhs);

  bool RangeInclusive(const larcaffe::Range_t& lhs, const larcaffe::Range_t& rhs);

  larcaffe::Range_t AND(const larcaffe::Range_t& lhs, const larcaffe::Range_t& rhs);

  larcaffe::Range_t OR(const larcaffe::Range_t& lhs, const larcaffe::Range_t& rhs);

  bool RangesOK( const RangeArray_t& ranges );

  void PrintRangeArray( const larcaffe::RangeArray_t& ranges );
  
}
#endif
