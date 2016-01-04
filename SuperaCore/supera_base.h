/**
 * \file supera_base.h
 *
 * \ingroup SupareCore
 * 
 * \brief Class definition file of supera_base
 *
 * @author Kazu - Nevis 2015
 */

/** \addtogroup SuperaCore

    @{*/

#ifndef __SUPERA_BASE_H__
#define __SUPERA_BASE_H__

#include <vector>
#include "SuperaTypes.h"
#include "supera_logger.h"

namespace larcaffe {

  namespace supera {
    
    /**
       \class supera_base
       Very base class of all everything else defined in this framework.
    */
    class supera_base {
      
    public:
      
      /// Default constructor
      supera_base(const std::string name="supera_base")
	: _name(name)
      { _logger = &(larcaffe::supera::logger::get(name)); }
      
      /// Default copy constructor
      supera_base(const supera_base &original) : _name(original._name){};
      
      /// Default destructor
      virtual ~supera_base(){};
      
      /// Name getter
      inline const std::string& name() const {return _name;};

      /// Logger getter
      inline const larcaffe::supera::logger& logger() const { return *_logger;}
      
    private:
      
      std::string _name;                ///< class name holder
      larcaffe::supera::logger *_logger; ///< logger
      
    };
  }
}
#endif

/** @} */ // end of doxygen group
