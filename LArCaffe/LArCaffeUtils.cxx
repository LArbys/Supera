
#include "LArCaffe/LArCaffeUtils.h"
#include "LArCaffe/larbys.h"

namespace larcaffe {

  bool RangeEmpty(const larcaffe::Range_t& r)
  { return (r.first < r.second); }

  bool RangeValidity(const larcaffe::Range_t& r)
  { return (r.first <= r.second); }

  bool RangeExclusive(const larcaffe::Range_t& lhs, const larcaffe::Range_t& rhs)
  { return (lhs.second < rhs.first || rhs.second < lhs.first); }

  bool RangeInclusive(const larcaffe::Range_t& lhs, const larcaffe::Range_t& rhs)
  { return (lhs.first < rhs.first && rhs.second < lhs.second); }

  larcaffe::Range_t AND(const larcaffe::Range_t& lhs, const larcaffe::Range_t& rhs)
  {
    if(!RangeValidity(lhs) || !RangeValidity(rhs)) throw larbys("<larcaffe::AND> Invalid range provided!");

    larcaffe::Range_t res(0,0);

    if(RangeExclusive(lhs,rhs)) return res;

    if(RangeInclusive(lhs,rhs)) res = lhs;
    else if(RangeInclusive(rhs,lhs)) res = rhs;
    else {
      res.first = std::min(lhs.first,rhs.first);
      res.second = std::max(lhs.second,rhs.second);
    }
    return res;
  }

  larcaffe::Range_t OR(const larcaffe::Range_t& lhs, const larcaffe::Range_t& rhs)
  {
    if(!RangeValidity(lhs) || !RangeValidity(rhs)) throw larbys("<larcaffe::AND> Invalid range provided!");

    larcaffe::Range_t res(0,0);
    res.first = std::min(lhs.first,rhs.first);
    res.second = std::max(lhs.second,rhs.second);
    return res;
  }

}
