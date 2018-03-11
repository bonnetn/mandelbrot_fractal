#include <iostream>
#include <ctime>
#include "lodepng/lodepng.h"
#include <thread>
#include <cmath>
#include "complex.hpp"
#include "threadpool.hpp"


namespace mandelbrot {
  template<typename T>
  bool mandelbrot(const T &c_real, const T &c_imag, const int max_iteration=30) {

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

  std::vector<bool> generate_picture(int width, int height)
  {
    constexpr auto chunk_size = 16;
    const auto cell_count = width*height;
    ThreadPool tp{std::thread::hardware_concurrency()};

    std::vector<bool> bits(cell_count);

    for(auto i{0}; i<cell_count/chunk_size; i++) {
      auto min = chunk_size*i;
      auto max = std::max(chunk_size*(i+1), cell_count-1);
      tp.addTask( [min,max,width,height,&bits]() {
        for(auto j=min; j<max; j++) {

          const auto x = j%width;
          const auto y = j/width;

          const auto coordX = (static_cast<double>(x)-width/2 )/height*2;
          const auto coordY = (static_cast<double>(y)-height/2 )/height*2;

          bits[j] = mandelbrot(coordX,coordY);
        }
      });
    }

    return bits;
  }

}

int main() {

  constexpr int width = 1366;
  constexpr int height = 768;

  const auto startTime = std::clock();
  const auto bits = mandelbrot::generate_picture(width, height);
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
