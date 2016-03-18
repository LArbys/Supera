#ifndef __LARCAFFETYPES_H__
#define __LARCAFFETYPES_H__

#include <string>
#include <utility>
#include <vector>
#include <limits>
#include <climits>
namespace larcaffe {

  const unsigned int kINVALID_UINT = std::numeric_limits<unsigned int>::max();

  class Range_t {
  public:
    Range_t( unsigned int start_, unsigned int end_ ) {
      start = start_;
      end = end_;
      filled = true;
    }
    Range_t() {
      start = kINVALID_UINT;
      end = 0;
      filled = false;
    }
    virtual ~Range_t() {};

    bool isFilled() const { return filled; };
    void setFilled() { filled = true; };
    unsigned int size() const { return std::abs((int)end-(int)start)+1; };

    unsigned int start;
    unsigned int end;
    bool filled = false;

  };

  typedef ::std::vector<Range_t> RangeArray_t;

  namespace msg {

    enum Level_t { kDEBUG, kINFO, kNORMAL, kWARNING, kERROR, kCRITICAL, kMSG_TYPE_MAX };

    const std::string kStringPrefix[kMSG_TYPE_MAX] =
      {
	"     \033[94m[DEBUG]\033[00m ",  ///< kDEBUG message prefix
	"      \033[92m[INFO]\033[00m ",  ///< kINFO message prefix
	"    \033[95m[NORMAL]\033[00m ",  ///< kNORMAL message prefix
	"   \033[93m[WARNING]\033[00m ", ///< kWARNING message prefix
	"     \033[91m[ERROR]\033[00m ", ///< kERROR message prefix
	"  \033[5;1;33;41m[CRITICAL]\033[00m "  ///< kCRITICAL message prefix
      };
    ///< Prefix of message
  }

}
#endif
