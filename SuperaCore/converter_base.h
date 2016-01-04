/**
 * \file supera_converter_base.h
 *
 * \ingroup SuperaCore
 * 
 * \brief Class def header for a class supera::converter_base
 *
 * @author kazuhiro
 */

/** \addtogroup SuperaCore

    @{*/
#ifndef __SUPERA_CONVERTER_BASE_H__
#define __SUPERA_CONVERTER_BASE_H__

#include <iostream>
#include "caffe.pb.h"
#include "supera_base.h"

namespace larcaffe {

  namespace supera {
    /**
       \class converter_base
       User defined class converter_base ... these comments are used to generate
       doxygen documentation!
    */
    class converter_base : public supera_base {
      
    public:
      
      /// Default constructor
      converter_base(const std::string name="converter_base");
      
      /// Default destructor
      ~converter_base(){}

      virtual void initialize() = 0;

      virtual void store() = 0;

      virtual void finalize() = 0;

      const ::caffe::Datum& data() const { return _data; }

      void set_image_size(const unsigned int   height,
			  const unsigned int   width);

      template <class T>
      void copy_data(const unsigned int channel,
		     const std::vector<T>& waveform,
		     const unsigned int waveform_index_to_start,
		     const unsigned int waveform_num_index_to_use,
		     const unsigned int adc_offset,
		     const unsigned int index_offset);

      void fill_zeros();

    private:

      ::caffe::Datum _data;

    };

    template<> void converter_base::copy_data<float>(const unsigned int,
						     const std::vector<float>&,
						     const unsigned int,
						     const unsigned int,
						     const unsigned int,
						     const unsigned int);
    template<> void converter_base::copy_data<short>(const unsigned int,
						     const std::vector<short>&,
						     const unsigned int,
						     const unsigned int,
						     const unsigned int,
						     const unsigned int);
    template<> void converter_base::copy_data<unsigned short>(const unsigned int,
							      const std::vector<unsigned short>&,
							      const unsigned int,
							      const unsigned int,
							      const unsigned int,
							      const unsigned int);
    
  }
}


#endif
/** @} */ // end of doxygen group 

