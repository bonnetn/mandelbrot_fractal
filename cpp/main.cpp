#include <iostream>
#include <ctime>
#include "lodepng/lodepng.h"
#include <thread>
#include <cmath>



#define WIDTH 400 // 1366
#define HEIGHT 400 // 768
#define MAX_ITERATION 30

namespace Complex {
  template<typename T>
  struct Complex {
      Complex(T const& a, T const& b) : real(a), imag(b) {}
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

template<typename T>
bool mandelbrot(T const& c_real, T const& c_imag) {

  const Complex::Complex<T> c(c_real, c_imag);
  Complex::Complex<T> z(c);

  for(auto i=0; i<MAX_ITERATION; i++) {
    Complex::square(z);
    z += c;
    if(Complex::norm2(z) > 4)
      return true; 
  }
  return false;
}

int main(void) {

  const auto startTime = std::clock();

  const auto bits = [&]() {

    const auto CELL_COUNT = WIDTH*HEIGHT;
    const auto THREAD_COUNT = static_cast<int>(std::thread::hardware_concurrency());
    const auto SLICE = static_cast<int>(std::ceil(CELL_COUNT/THREAD_COUNT));

    std::vector<bool> bits(CELL_COUNT);
    std::vector<std::thread> threads(THREAD_COUNT);

    for(auto threadIndex=0; threadIndex < THREAD_COUNT; threadIndex++) {
      threads[threadIndex] = std::thread( [&bits](int min, int max) {
        for(auto j=min; j<max; j++) {

          const auto x = j%WIDTH;
          const auto y = j/WIDTH;

          const auto coordX = (static_cast<double>(x)-WIDTH/2 )/HEIGHT*2;
          const auto coordY = (static_cast<double>(y)-HEIGHT/2 )/HEIGHT*2;

          bits[j] = mandelbrot(coordX,coordY);
       
        }
      }, threadIndex*SLICE, std::min(SLICE*(threadIndex+1), CELL_COUNT));
    }

    for( auto & t : threads ) {
      t.join();
    }
    return bits;
  }();


  std::cout << "Time: " << (std::clock()-startTime) * (static_cast<double>(1000) / CLOCKS_PER_SEC) << "ms" << std::endl;
  
  const auto pixels = [&]() {
    std::vector<unsigned char> pixels(4 * bits.size());

    for(decltype(bits)::size_type i(0); i<bits.size(); i++) {
      const unsigned char val = bits[i] ? 0 : 255;
      pixels[4 * i + 0]  = val;
      pixels[4 * i + 1]  = val;
      pixels[4 * i + 2]  = val;
      pixels[4 * i + 3]  = 255;
    }

    return pixels;
  }();
  const auto error = lodepng::encode("mandelbrot.png", pixels, WIDTH, HEIGHT);

  // if there's an error, display it
  if(error) 
    std::cout << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;



}
