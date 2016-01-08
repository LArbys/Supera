/**
 * \file larbys.h
 *
 * \ingroup LArCaffe
 * 
 * \brief Class def header for exception classes in LArUtil package
 *
 * @author kazuhiro
 */

/** \addtogroup LArCaffe

    @{*/
#ifndef __LARBYS_H__
#define __LARBYS_H__

#include <iostream>
#include <exception>

namespace larcaffe {

  /**
     \class larbys
     Generic (base) exception class
  */
  class larbys : public std::exception {
    
  public:
    
    larbys(std::string msg="") : std::exception()
    {
      _msg = "\033[93m";
      _msg += msg;
      _msg += "\033[00m";
    }
    
    virtual ~larbys() throw(){};
    virtual const char* what() const throw() 
    { return _msg.c_str(); }
    
  private:
    
    std::string _msg;
  };
}
#endif
/** @} */ // end of doxygen group 

