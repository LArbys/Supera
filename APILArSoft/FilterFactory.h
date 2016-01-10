#ifndef __FILTERFACTORY__
#define __FILTERFACTORY__

namespace larcaffe {
  namespace supera {

    class FilterBase;

    class FilterFactory {
    public:
      virtual FilterBase* create() = 0;
    };

  }
}

#endif
