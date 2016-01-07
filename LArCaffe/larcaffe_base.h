/**
 * \file larcaffe_base.h
 *
 * \ingroup LArCaffe
 * 
 * \brief Class definition file of larcaffe_base
 *
 * @author Kazu - Nevis 2015
 */

/** \addtogroup LArCaffe

    @{*/

#ifndef __LARCAFFE_BASE_H__
#define __LARCAFFE_BASE_H__

#include <vector>
#include "LArCaffe/larcaffe_logger.h"

namespace larcaffe {
    
  /**
     \class larcaffe_base
     Very base class of all everything else defined in this framework.
  */
  class larcaffe_base {
    
  public:
    
    /// Default constructor
    larcaffe_base(const std::string name="larcaffe_base")
      : _name(name)
    { _logger = &(larcaffe::logger::get(name)); }
    
    /// Default copy constructor
    larcaffe_base(const larcaffe_base &original) : _name(original._name){};
    
    /// Default destructor
    virtual ~larcaffe_base(){};
    
    /// Name getter
    inline const std::string& name() const {return _name;};
    
    /// Logger getter
    inline const larcaffe::logger& logger() const { return *_logger;}
    
    /// Verbosity level
    void set_verbosity(larcaffe::msg::Level_t level) { _logger->set(level); }
    
  private:
    
    std::string _name;                ///< class name holder
    larcaffe::logger *_logger; ///< logger
    
  };
}
#endif

/** @} */ // end of doxygen group
