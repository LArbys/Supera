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
#include "LArCaffe/larcaffe_base.h"
#include "LArCaffe/Image.h"
namespace caffe {
  class Datum;
}

#ifndef __CINT__
#include "caffe.pb.h"
#endif

namespace larcaffe {

  namespace supera {
    /**
       \class converter_base
       User defined class converter_base ... these comments are used to generate
       doxygen documentation!
    */
    class converter_base : public larcaffe_base {
      
    public:
      
      /// Default constructor
      converter_base(const std::string name="converter_base");
      
      /// Default destructor
      ~converter_base(){}

      virtual void initialize() = 0;

      virtual void store_image(const std::string& key) = 0;

      virtual void write() = 0;

      virtual void finalize() = 0;

      const ::caffe::Datum& data() const { return _data; }

      void set_image_size(const unsigned int   height,
			  const unsigned int   width);

      void copy_data(const unsigned int channel,
		     const std::vector<short>& waveform,
		     const unsigned int waveform_index_to_start,
		     const unsigned int waveform_num_index_to_use,
		     const unsigned int adc_offset,
		     const unsigned int index_offset);

      void copy_data(const unsigned int channel,
		     const std::vector<float>& waveform,
		     const unsigned int waveform_index_to_start,
		     const unsigned int waveform_num_index_to_use,
		     const unsigned int adc_offset,
		     const unsigned int index_offset);

      void copy_data(const Image& img);		     

      void fill_zeros();

    protected:

      ::caffe::Datum _data;

    };

  }
}


#endif
/** @} */ // end of doxygen group 

