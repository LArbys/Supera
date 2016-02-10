#ifndef __FILTERFACTORY_H__
#define __FILTERFACTORY_H__

#include "LArCaffe/larbys.h"
#include <string>
#include <map>

namespace larcaffe {
  namespace supera {
    
    class FilterBase;
    
    class FilterFactoryBase {
    public:
      FilterFactoryBase(){}
      virtual ~FilterFactoryBase(){}
      virtual FilterBase* create() = 0;
    };
    
    class FilterFactory {
    public:
      FilterFactory(){}
      ~FilterFactory(){}
      static FilterFactory* get() {
	if(!_me) _me = new FilterFactory;
	return _me;
      }
      FilterBase* create(std::string type_name) {
	auto iter = _factory.find(type_name);
	if(iter == _factory.end()) {
	  std::cerr << "\033[93mUnknown filter type requested...\033[00m" << std::endl;
	  return nullptr;
      }
	return (*iter).second->create();
      }
      
      void add_factory(std::string type_name,FilterFactoryBase* factory) {
	if(!factory) return;
	auto iter = _factory.find(type_name);
	if(iter != _factory.end()) {
	  std::cerr<<"\033[93mDuplicate factory registration for " << type_name << std::endl;
	  throw larbys("Duplicate factory registration...");
	}
	_factory[type_name]=factory;
      }
      
    private:      
      static FilterFactory* _me;
      std::map<std::string,::larcaffe::supera::FilterFactoryBase*> _factory;
    };
  }
}

#endif
