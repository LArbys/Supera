#ifndef __SUPERA_CONVERTER_BASE_CXX__
#define __SUPERA_CONVERTER_BASE_CXX__

#include "converter_base.h"
#include "LArCaffe/larbys.h"
namespace larcaffe {

  namespace supera {

    converter_base::converter_base(const std::string name) : larcaffe_base(name)
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
      for(size_t i=0; i<(size_t)(_data.height()*_data.width()); ++i) {
	//_data.add_float_data(i,0.);
	_data.add_float_data(0.);
      }
    }

    void converter_base::copy_data(const unsigned int wire,
				   const std::vector<short>& waveform,
				   const unsigned int waveform_index_to_start,
				   const unsigned int waveform_num_index_to_use,
				   const unsigned int adc_offset,
				   const unsigned int index_offset)
    {

      const unsigned int num_samples = (waveform_num_index_to_use ? waveform_num_index_to_use : waveform.size());

      if( logger().debug() )
	logger().LOG(msg::kDEBUG,__FUNCTION__,__LINE__)
	  << "Adding data for wire " << wire << " (length=" << waveform.size() << ") "
	  << "for index " << waveform_index_to_start << " => " << waveform_index_to_start + waveform_num_index_to_use
	  << " ... with an offset " << adc_offset << " placed with index offset " << index_offset << std::endl;
      
      // sanity check (size of image, etc)
      if( waveform_index_to_start >= waveform.size() ) {
	logger().LOG(msg::kCRITICAL,__FUNCTION__,__LINE__)
	  << "Requested to skip " << waveform_index_to_start << " for a waveform of length " << waveform.size() << std::endl;
	throw larbys();
      }

      if( num_samples > (waveform.size() - waveform_index_to_start) ) {
	logger().LOG(msg::kCRITICAL,__FUNCTION__,__LINE__)
	  << "Cannot take " << num_samples << " samples for a waveform of length " << waveform.size()
	  << " starting from index " << waveform_index_to_start << std::endl;
	throw larbys();
      }
	  
      if( _data.width() <= wire ) {
	logger().LOG(msg::kCRITICAL,__FUNCTION__,__LINE__)
	  << "Wire " << wire << " cannot fit in defined data size (width=" << _data.width() << ")" << std::endl;
	throw larbys();
      }
      
      if( (_data.height() * _data.width()) < (wire * _data.height() + index_offset + num_samples) ) {
	logger().LOG(msg::kCRITICAL,__FUNCTION__,__LINE__)
	  << "Requested to fill size " << waveform.size() << " from " << (wire * _data.height() + index_offset)
	  << " ... impossible as image size = " << (_data.height() * _data.width()) << std::endl;
	throw larbys();
      }

      const size_t total_index_offset = wire * _data.height() + index_offset;
      
      for(size_t index=0; index<num_samples; ++index)

	_data.set_float_data( (total_index_offset + index), (float)(waveform[index + waveform_index_to_start]) );
      
    }

    void converter_base::copy_data(const unsigned int wire,
				   const std::vector<float>& waveform,
				   const unsigned int waveform_index_to_start,
				   const unsigned int waveform_num_index_to_use,
				   const unsigned int adc_offset,
				   const unsigned int index_offset)
    {

      const unsigned int num_samples = (waveform_num_index_to_use ? waveform_num_index_to_use : waveform.size());

      if( logger().debug() )
	logger().LOG(msg::kDEBUG,__FUNCTION__,__LINE__)
	  << "Adding data for wire " << wire << " (length=" << waveform.size() << ") "
	  << "for index " << waveform_index_to_start << " => " << waveform_index_to_start + waveform_num_index_to_use
	  << " ... with an offset " << adc_offset << " placed with index offset " << index_offset << std::endl;
      
      // sanity check (size of image, etc)
      if( waveform_index_to_start >= waveform.size() ) {
	logger().LOG(msg::kCRITICAL,__FUNCTION__,__LINE__)
	  << "Requested to skip " << waveform_index_to_start << " for a waveform of length " << waveform.size() << std::endl;
	throw larbys();
      }

      if( num_samples > (waveform.size() - waveform_index_to_start) ) {
	logger().LOG(msg::kCRITICAL,__FUNCTION__,__LINE__)
	  << "Cannot take " << num_samples << " samples for a waveform of length " << waveform.size()
	  << " starting from index " << waveform_index_to_start << std::endl;
	throw larbys();
      }
	  
      if( _data.width() <= wire ) {
	logger().LOG(msg::kCRITICAL,__FUNCTION__,__LINE__)
	  << "Wire " << wire << " cannot fit in defined data size (width=" << _data.width() << ")" << std::endl;
	throw larbys();
      }
      
      if( (_data.height() * _data.width()) < (wire * _data.height() + index_offset + num_samples) ) {
	logger().LOG(msg::kCRITICAL,__FUNCTION__,__LINE__)
	  << "Requested to fill size " << waveform.size() << " from " << (wire * _data.height() + index_offset)
	  << " ... impossible as image size = " << (_data.height() * _data.width()) << std::endl;
	throw larbys();
      }

      const size_t total_index_offset = wire * _data.height() + index_offset;
      
      for(size_t index=0; index<num_samples; ++index)

	_data.set_float_data( (total_index_offset + index), (float)(waveform[index + waveform_index_to_start]) );
      
    }
  }

}

#endif
