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
      Complex(T const& a, T const& b) : m_real(a), m_imag(b) {}

      T m_real;
      T m_imag;
  };

  template<typename T>
  void square( Complex<T>& a ) {
    // (a+bi)(a+bi) = a2 + 2abi - b2
    const T temp = a.m_imag;
    a.m_imag = 2 * a.m_imag * a.m_real;
    a.m_real = a.m_real*a.m_real - temp * temp;
  }

  template<typename T>
  Complex<T>& operator+=( Complex<T>& a, Complex<T> const& b ) {
      a.m_real += b.m_real;
      a.m_imag += b.m_imag;
      return a;
  }

  template<typename T>
  T norm2(Complex<T> const& z) {
    return z.m_imag*z.m_imag + z.m_real*z.m_real;
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

  auto startTime = std::clock();

  int THREAD_COUNT = std::thread::hardware_concurrency();
  int CELL_COUNT = WIDTH*HEIGHT;
  int SLICE = std::ceil(CELL_COUNT/THREAD_COUNT);

  std::vector<std::thread> threads(THREAD_COUNT);
  std::vector<bool> bits(CELL_COUNT);

  for(auto threadIndex=0; threadIndex < THREAD_COUNT; threadIndex++) {
    threads[threadIndex] = std::thread( [&bits](int min, int max) {
      for(auto j=min; j<max; j++) {

        double x = j%WIDTH;
        double y = j/WIDTH;

        auto coordX = (x-WIDTH/2 )/HEIGHT*2;
        auto coordY = (y-HEIGHT/2 )/HEIGHT*2;

        bits[j] = mandelbrot(coordX,coordY);
     
      }
    }, threadIndex*SLICE, std::min(SLICE*(threadIndex+1), CELL_COUNT));
  }

  for( auto & t : threads ) {
    t.join();
  }


  std::cout << "Time: " << (std::clock()-startTime) * ((double) 1000 / CLOCKS_PER_SEC) << "ms" << std::endl;
  // Encode the image
  std::vector<unsigned char> pixels(sizeof(unsigned char) * 4 * CELL_COUNT);

  for(auto i(0); i<CELL_COUNT; i++) {
    unsigned char val = bits[i] ? 0 : 255;
    pixels[4 * i + 0]  = val;
    pixels[4 * i + 1]  = val;
    pixels[4 * i + 2]  = val;
    pixels[4 * i + 3]  = 255;
  }
  auto error = lodepng::encode("mandelbrot.png", pixels, WIDTH, HEIGHT);

  // if there's an error, display it
  if(error) 
    std::cout << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;



}
