#ifndef __SUPERA_CONVERTER_BASE_CXX__
#define __SUPERA_CONVERTER_BASE_CXX__

#include "converter_base.h"
#include "supera_exception.h"
namespace larcaffe {

  namespace supera {

    converter_base::converter_base(const std::string name) : supera_base(name)
    {}

    void converter_base::set_image_size(const unsigned int   height,
					const unsigned int   width)
    {
      _data.set_channels ( 1      ); // gray scale
      _data.set_height   ( height );
      _data.set_width    ( width  );
      _data.set_label    ( 0      ); // TODO what is this?
      fill_zeros();
    }

    void converter_base::fill_zeros()
    {
      for(size_t i=0; i<(_data.height()*_data.width()); ++i)
	_data.set_float_data(0,i);
    }

    template <class T>
    void converter_base::copy_data(const unsigned int channel,
				   const std::vector<T>& waveform,
				   const unsigned int waveform_index_to_start,
				   const unsigned int waveform_num_index_to_use,
				   const unsigned int adc_offset,
				   const unsigned int index_offset)
    {

      const unsigned int num_samples = (waveform_num_index_to_use ? waveform_num_index_to_use : waveform.size());
      
      // sanity check (size of image, etc)
      if( waveform_index_to_start >= waveform.size() ) {
	logger().LOG(msg::kCRITICAL,__FUNCTION__,__LINE__)
	  << "Requested to skip " << waveform_index_to_start << " for a waveform of length " << waveform.size() << std::endl;
	throw supera_exception();
      }

      if( num_samples > (waveform.size() - waveform_index_to_start) ) {
	logger().LOG(msg::kCRITICAL,__FUNCTION__,__LINE__)
	  << "Cannot take " << num_samples << " samples for a waveform of length " << waveform.size()
	  << " starting from index " << waveform_index_to_start << std::endl;
	throw supera_exception();
      }
	  
      if( _data.width() <= channel ) {
	logger().LOG(msg::kCRITICAL,__FUNCTION__,__LINE__)
	  << "Channel " << channel << " cannot fit in defined data size (width=" << _data.width() << ")" << std::endl;
	throw supera_exception();
      }
      
      if( (_data.height() * _data.width()) <= (channel * _data.height() + index_offset + num_samples) ) {
	logger().LOG(msg::kCRITICAL,__FUNCTION__,__LINE__)
	  << "Requested to fill size " << waveform.size() << " from " << (channel * _data.height() + index_offset)
	  << " ... impossible as image size = " << (_data.height() * _data.width()) << std::endl;
	throw supera_exception();
      }

      const size_t total_index_offset = channel * _data.height() + index_offset;
      
      for(size_t index=0; index<num_samples; ++index)

	_data.set_float_data( (float)(waveform[index + waveform_index_to_start]), (total_index_offset + index) );
      
    }

  }

}

#endif
