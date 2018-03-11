#ifndef COMPLEX_HPP_
#define COMPLEX_HPP_

namespace complex {
  template<typename T>
  struct Complex {
      Complex(const T& a, const T& b) : real{a}, imag{b} {}
      Complex() = default;

      T real{0};
      T imag{0};

      auto& operator+=(const Complex<T> &b) {
          real += b.real;
          imag += b.imag;
          return *this;
      }
  };

  template<typename T>
  void square(Complex<T> &a) {
    // (a+bi)(a+bi) = a2 + 2abi - b2
    const T temp = a.imag;
    a.imag = 2 * a.imag * a.real;
    a.real = a.real*a.real - temp * temp;
  }


  template<typename T>
  auto norm2(const Complex<T> &z) {
    return z.imag*z.imag + z.real*z.real;
  }
}
#endif
