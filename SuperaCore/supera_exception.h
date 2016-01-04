/**
 * \file supera_exception.h
 *
 * \ingroup LArUtil
 * 
 * \brief Class def header for exception classes in LArUtil package
 *
 * @author kazuhiro
 */

/** \addtogroup LArUtil

    @{*/
#ifndef __SUPERA_EXCEPTION_H__
#define __SUPERA_EXCEPTION_H__

#include <iostream>
#include <exception>

namespace larcaffe {

  namespace supera {
    /**
       \class supera_exception
       Generic (base) exception class
    */
    class supera_exception : public std::exception{
      
    public:
      
      supera_exception(std::string msg="") : std::exception()
      {
	_msg = "\033[93m";
	_msg += msg;
	_msg += "\033[00m";
      }
      
      virtual ~supera_exception() throw(){};
      virtual const char* what() const throw() 
      { return _msg.c_str(); }
      
    private:
      
      std::string _msg;
    };
  }
}
#endif
/** @} */ // end of doxygen group 

