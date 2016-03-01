
#include "LArCaffe/LArCaffeUtils.h"
#include "LArCaffe/larbys.h"

namespace larcaffe {

  bool RangeEmpty(const larcaffe::Range_t& r)
  { return r.isFilled(); }

  bool RangeValidity(const larcaffe::Range_t& r)
  { return (r.start <= r.end); }

  bool RangeExclusive(const larcaffe::Range_t& lhs, const larcaffe::Range_t& rhs)
  { return (lhs.end < rhs.start || rhs.end < lhs.start); }

  bool RangeInclusive(const larcaffe::Range_t& lhs, const larcaffe::Range_t& rhs)
  { return (lhs.start < rhs.start && rhs.end < lhs.end); }

  larcaffe::Range_t AND(const larcaffe::Range_t& lhs, const larcaffe::Range_t& rhs)
  {
    if(!RangeValidity(lhs) || !RangeValidity(rhs)) throw larbys("<larcaffe::AND> Invalid range provided!");

    larcaffe::Range_t res(0,0);

    if(RangeExclusive(lhs,rhs)) return res;

    if(RangeInclusive(lhs,rhs)) res = lhs;
    else if(RangeInclusive(rhs,lhs)) res = rhs;
    else {
      res.start = std::min(lhs.start,rhs.start);
      res.end = std::max(lhs.end,rhs.end);
    }
    return res;
  }

  larcaffe::Range_t OR(const larcaffe::Range_t& lhs, const larcaffe::Range_t& rhs)
  {
    if(!RangeValidity(lhs) || !RangeValidity(rhs)) throw larbys("<larcaffe::AND> Invalid range provided!");

    larcaffe::Range_t res(0,0);
    res.start = std::min(lhs.start,rhs.start);
    res.end = std::max(lhs.end,rhs.end);
    return res;
  }

}
