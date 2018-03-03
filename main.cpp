#include <iostream>
#include <ctime>
#include "lodepng/lodepng.h"



#define WIDTH 1366
#define HEIGHT 768
#define MAX_ITERATION 100

template<typename T>
class Complex {
  public: 
    Complex(T const& a, T const& b) : m_real(a), m_imag(b) {}
    Complex(Complex<T> const& a) {
      m_real = a.getReal();
      m_imag = a.getImag();
    }

    void square(void) {
      // (a+bi)(a+bi) = a2 + 2abi - b2
      const T temp = m_imag;
      m_imag = 2 * m_imag * m_real;
      m_real = m_real*m_real - temp * temp;
    }

    void add(Complex<T> const& c) {
      m_real += c.getReal();
      m_imag += c.getImag();
    }

    T getReal() const {
      return m_real;
    }

    T getImag() const {
      return m_imag;
    }

    T norm2() const {
      return m_imag*m_imag + m_real*m_real;
    }

  private:
    T m_real;
    T m_imag;
};

template<typename T>
bool mandelbrot(T const& c_real, T const& c_imag) {

  const Complex<T> c(c_real, c_imag);
  Complex<T> z(c);

  for(auto i=0; i<MAX_ITERATION; i++) {
    z.square();
    z.add(c);
    if(z.norm2() >= 4)
      return true; 
  }
  return false;
}

int main(void) {

  std::vector<unsigned char> pixels(sizeof(unsigned char) * 4 * WIDTH*HEIGHT);
  auto startTime = std::clock();

  for(auto x=0; x<WIDTH; x++)
  for(auto y=0; y<HEIGHT; y++) {

    auto coordX = (((double) x)-WIDTH/2 )/HEIGHT*2;
    auto coordY = (((double) y)-HEIGHT/2 )/HEIGHT*2;

    unsigned char val = mandelbrot(coordX,coordY) ? 0 : 255;
    for( int i=0; i<3; i++ )
      pixels[4 * x + 4 * WIDTH * y + i]  = val;
    pixels[4 * x + 4 * WIDTH * y + 3]  = 255;
  }

  std::cout << "Time: " << (std::clock()-startTime) * ((double) 1000 / CLOCKS_PER_SEC) << "ms" << std::endl;

  //Encode the image
  auto error = lodepng::encode("mandelbrot.png", pixels, WIDTH, HEIGHT);

  //if there's an error, display it
  if(error) 
    std::cout << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;



}
