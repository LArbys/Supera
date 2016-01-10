#ifndef __FILTER_EMPTY_IMAGES__
#define __FILTER_EMPTY_IMAGES__

#include "FilterBase.h"

namespace larcaffe {

  namespace supera {

    class FilterEmptyImages : public FilterBase {
      
    public:

      FilterEmptyImages();
      virtual ~FilterEmptyImages();
      
      // required
      virtual std::string name() const { return "FilterEmptyImages"; };
      virtual void configure() {};
      virtual bool doWeKeep( const larcaffe::supera::converter_base&  );
      
    };


    class FilterEmptyImagesFactory : public FilterFactory {
    public:
      FilterEmptyImagesFactory() {
	FilterBase::registerConcreteFactory( "FilterEmptyImages", this );
      };
      ~FilterEmptyImagesFactory() {};
      
      virtual FilterBase* create() { 
	return new FilterEmptyImages;
      };
    };
      

  }

}

#endif
