#ifndef __CONVERTER_API__
#define __CONVERTER_API__

#include "RawData/RawDigit.h"
#include "RecoBase/Wire.h"
#include "RecoBase/Hit.h"
#include "SuperaCore/converter_base.h"
#include <utility>

namespace larcaffe {

  namespace supera {

    typedef std::pair<unsigned int, unsigned int> Range_t;

    typedef std::vector<Range_t> RangeArray_t;

    class ConverterAPI : public larcaffe_base {

    public:

      ConverterAPI() : larcaffe_base("ConverterAPI") {}

      ~ConverterAPI() {}

      void SetRange(int min, int max, unsigned int plane_id);

      const RangeArray_t& Ranges() const { return _range_v; }

      bool InRange(const ::geo::WireID& wid, const unsigned int time) const;

      bool InRange(const ::geo::WireID& wid) const;

      void Copy(const std::vector<raw::RawDigit>&, larcaffe::supera::converter_base&) const;

      void Copy(const std::vector<recob::Wire>&, larcaffe::supera::converter_base&) const;

      void Copy(const std::vector<recob::Hit>&, larcaffe::supera::converter_base&) const;

    protected:

      RangeArray_t _range_v;

    };
  }
}
#endif
