#ifndef __SUPERATYPES_H__
#define __SUPERATYPES_H__

#include <string>

namespace larcaffe {

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
  };

}
#endif
