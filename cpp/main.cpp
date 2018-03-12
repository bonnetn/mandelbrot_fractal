#include <iostream>
#include <ctime>
#include "lodepng/lodepng.h"
#include <thread>
#include <cmath>
#include "complex.hpp"
#include "threadpool.hpp"


class MandelbrotGenerator {
  public:
  virtual std::vector<bool> generate_picture() = 0;
};

namespace Impl {
  template<typename T>
  bool mandelbrot(const T &c_real, const T &c_imag, const int max_iteration=60) {

    const complex::Complex<T> c{c_real, c_imag};
    complex::Complex<T> z{c};

    for(auto i=0; i!=max_iteration; i++) {
      complex::square(z);
      z += c;
      if(complex::norm2(z) > 4)
        return true; 
    }
    return false;
  }

  class MandelbrotGenerator : public ::MandelbrotGenerator {

    private:
    auto generateMandelbrotSlice(size_t min, size_t max)
    {
      return [&,min,max]() {
        for(auto j=min; j<max; j++) {

          const auto x = j%m_width;
          const auto y = j/m_width;

          const auto coordX = (static_cast<double>(x)-m_width/2 )/m_height*2;
          const auto coordY = (static_cast<double>(y)-m_height/2 )/m_height*2;

          m_bits[j] = mandelbrot(coordX,coordY);
        }
      };
    }

    public:
    MandelbrotGenerator(size_t width = 400, size_t height = 400) : 
      m_width{width}, m_height{height}, m_bits(width*height)
    {
    }

    virtual std::vector<bool> generate_picture()
    {
      const auto cell_count = m_width*m_height;
      const auto chunk_count = cell_count/chunk_size+1;

      for(auto i{0u}; i<chunk_count; i++) {
        const auto min = chunk_size*i;
        const auto max = std::min(chunk_size*(i+1), cell_count-1);
        m_tp.addTask(generateMandelbrotSlice(min, max));
      }

      m_tp.waitForTasksFinished();

      return m_bits;
    }

    private:
    static constexpr auto chunk_size = 512lu;
    size_t m_width{400};
    size_t m_height{400};
    ThreadPool m_tp{std::thread::hardware_concurrency()};
    std::vector<bool> m_bits;

  };
}

int main() {

  constexpr int width = 1366;
  constexpr int height = 768;

  const auto startTime = std::clock();
  Impl::MandelbrotGenerator generator{width, height};
  const auto bits = generator.generate_picture();
  std::cout << "Time: " << (std::clock()-startTime) * (static_cast<double>(1000) / CLOCKS_PER_SEC) << "ms" << std::endl;
  
  const auto pixels = [&bits]() {
    std::vector<unsigned char> pixels;
    pixels.reserve(4 * bits.size());

    for(auto b : bits) {
      const unsigned char pxValue = b ? 0 : 255;
      std::vector<unsigned char> px{pxValue,pxValue,pxValue,255};
      pixels.insert(end(pixels), begin(px), end(px));
    }

    return pixels;
  }();

  lodepng::encode("mandelbrot.png", pixels, width, height);

}
