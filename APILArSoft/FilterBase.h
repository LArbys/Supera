/**
 * \file FilterBase.h
 *
 * \ingroup LArCaffe
 * 
 * \brief Interface class for image filter
 *
 * @author T. Wongjirad
 */

/** \addtogroup LArCaffe

    @{*/

#ifndef __FILTER_BASE__
#define __FILTER_BASE__

#include <string>
#include <map>
#include "fhiclcpp/ParameterSet.h"
#include "SuperaCore/converter_base.h"
#include "ConverterAPI.h"
#include "FilterFactory.h"
#include "LArCaffe/larbys.h"

namespace larcaffe {

  namespace supera {
    
    class FilterBase {
      
    public:
      FilterBase() {};
      virtual ~FilterBase() {};

      // both functions called by Supera_module before storing image
      virtual std::string name() const = 0;        //< return identify for type of filter
      virtual void configure( fhicl::ParameterSet const & p ) = 0;                //< user function to configure class before applying filter -- called by Supera_module
      virtual bool doWeKeep( const ::larcaffe::supera::converter_base&  ) = 0; //< apply filter -- called by Supera_module

    };

  }
}

#endif
