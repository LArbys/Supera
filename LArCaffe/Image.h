/**
 * \file larbys.h
 *
 * \ingroup LArCaffe
 * 
 * \brief Class def header for exception classes in LArUtil package
 *
 * @author tmw, kazu
 */

/** \addtogroup LArCaffe

    @{*/

#ifndef __Image__
#define __Image__

#include <vector>
#include <cstdlib>
namespace larcaffe {
  
  class Image : protected std::vector<float> {
    
  public:
    
    Image(size_t height=0, size_t width=0);
    Image(const Image& rhs);
    virtual ~Image(){}

    unsigned int height() const { return fHeight; }
    unsigned int width()  const { return fWidth;  }
    float pixel(size_t h, size_t w) const;
    inline bool  isInBounds( size_t h, size_t w ) const
    { return ( h < fHeight && w < fWidth ); }
    Image copy_compress(size_t height, size_t width) const;
    size_t index( size_t height, size_t width ) const;

    void copy(size_t h, size_t w, const float* src, size_t num_pixel);
    void copy(size_t h, size_t w, const std::vector<float>& src, size_t num_pixel=0);
    void copy(size_t h, size_t w, const short* src, size_t num_pixel);
    void copy(size_t h, size_t w, const std::vector<short>& src, size_t num_pixel=0);
    const std::vector<float>& as_vector() const 
    { return (*this); }
    void resize( size_t height, size_t width );
    void set_pixel( size_t h, size_t w, float value );
    void paint(float value);
    void clear_data();
    void compress(size_t height, size_t width);

  private:
    unsigned int fHeight;
    unsigned int fWidth;
    void clear();
  };
}

#endif
/** @} */ // end of doxygen group 
