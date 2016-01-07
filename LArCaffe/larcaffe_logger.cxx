#ifndef __LARCAFFELOGGER_CXX__
#define __LARCAFFELOGGER_CXX__

#include "larcaffe_logger.h"
namespace larcaffe {

  std::map<std::string,logger> *logger::_logger_m = nullptr;
  
  std::ostream& logger::LOG(const msg::Level_t level) const
  {
    (*_ostrm)  << msg::kStringPrefix[level].c_str()
	       << "\033[0m ";
    return (*_ostrm);
  }
  
  std::ostream& logger::LOG(const msg::Level_t level,
			    const std::string& function ) const
  {
    auto& strm(LOG(level));
    strm << "\033[95m<" << function.c_str() << ">\033[00m ";
    return strm;
  }
  
  std::ostream& logger::LOG(const msg::Level_t level,
			    const std::string& function,
			    const unsigned int line_num ) const
  {
    auto& strm(LOG(level));
    strm << "\033[95m<" << function.c_str() << "::L" << line_num << ">\033[00m ";
    return strm;
  }
  
  std::ostream& logger::LOG(const msg::Level_t level,
			    const std::string& function,
			    const unsigned int line_num,
			    const std::string& file_name) const
  {
    auto& strm(LOG(level,function));
    strm << file_name.c_str() << "::L" << line_num << " ";
    return strm;
  }
}
#endif
