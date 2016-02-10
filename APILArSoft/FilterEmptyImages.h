#ifndef __FILTER_EMPTY_IMAGES__
#define __FILTER_EMPTY_IMAGES__

#include "FilterBase.h"
#include "TH1D.h"

namespace larcaffe {

  namespace supera {

    class FilterEmptyImages : public FilterBase {
      
    public:

      FilterEmptyImages();
      virtual ~FilterEmptyImages();
      
      // required
      virtual std::string name() const { return "FilterEmptyImages"; };
      virtual void configure( fhicl::ParameterSet const & p );
      virtual bool doWeKeep( const ::larcaffe::supera::converter_base& img_data  );
      
      TH1D* hadcs;

      // Configuration Parameters
      int sigmaToCheck;
      double ratioThreshold;
      
      
    };

    class FilterEmptyImagesFactory : public FilterFactoryBase {
    public:
      FilterEmptyImagesFactory(){ FilterFactory::get()->add_factory("FilterEmptyImages",this); }
      ~FilterEmptyImagesFactory(){}
      FilterBase* create()
      {return new FilterEmptyImages;}
    };

  }

}

#endif
