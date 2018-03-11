#include <iostream>
#include <ctime>
#include "lodepng/lodepng.h"
#include <thread>
#include <cmath>



namespace Complex {
  template<typename T>
  struct Complex {
      Complex(const T& a, const T& b) : real(a), imag(b) {}
      Complex() = default;

      T real = 0;
      T imag = 0;
  };

  template<typename T>
  auto square( Complex<T>& a ) {
    // (a+bi)(a+bi) = a2 + 2abi - b2
    const T temp = a.imag;
    a.imag = 2 * a.imag * a.real;
    a.real = a.real*a.real - temp * temp;
  }

  template<typename T>
  auto& operator+=( Complex<T>& a, Complex<T> const& b ) {
      a.real += b.real;
      a.imag += b.imag;
      return a;
  }

  template<typename T>
  auto norm2(Complex<T> const& z) {
    return z.imag*z.imag + z.real*z.real;
  }
}

namespace Mandelbrot {
  template<typename T>
  bool mandelbrot(T const& c_real, T const& c_imag, const int max_iteration=30) {

    const Complex::Complex<T> c(c_real, c_imag);
    Complex::Complex<T> z(c);

    for(auto i=0; i!=max_iteration; i++) {
      Complex::square(z);
      z += c;
      if(Complex::norm2(z) > 4)
        return true; 
    }
    return false;
  }

  auto generate_picture(int width, int height) {
    const auto cell_count = width*height;
    const auto thread_count = static_cast<int>(std::thread::hardware_concurrency());
    const auto slice = static_cast<int>(std::ceil(cell_count/thread_count));

    std::vector<bool> bits(cell_count);
    std::vector<std::thread> threads(thread_count);

    for(auto threadIndex=0; threadIndex < thread_count; threadIndex++) {
      threads[threadIndex] = std::thread( [&bits,width,height](int min, int max) {
        for(auto j=min; j<max; j++) {

          const auto x = j%width;
          const auto y = j/width;

          const auto coordX = (static_cast<double>(x)-width/2 )/height*2;
          const auto coordY = (static_cast<double>(y)-height/2 )/height*2;

          bits[j] = mandelbrot(coordX,coordY);
       
        }
      }, threadIndex*slice, std::min(slice*(threadIndex+1), cell_count));
    }

    for( auto & t : threads ) {
      t.join();
    }
    return bits;
  }

}

int main(void) {

  constexpr int width = 1366;
  constexpr int height = 768;

  const auto startTime = std::clock();
  const auto bits = Mandelbrot::generate_picture(width, height);
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
