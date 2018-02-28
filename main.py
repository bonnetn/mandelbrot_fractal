#!python3

import numpy as np
import numba
from PIL import Image
import time


WIDTH = 1366
HEIGHT = 768

HALF_WIDTH = np.floor(WIDTH/2)
HALF_HEIGHT = np.floor(HEIGHT/2)

TARGET = "parallel"
ITERATION_COUNT = 30


@numba.vectorize(['boolean(complex128,int64,int64)'], target=TARGET)
def mandelbrot(a, x, y):
    """
    Computes Zn = Zn-1 + C and see if it diverges
    """

    x = x - HALF_WIDTH
    y = y - HALF_HEIGHT

    c = complex(x, y) / HALF_HEIGHT

    for i in range(ITERATION_COUNT):
        a = np.square(a) + c
        if a.real**2 + a.imag**2 >= 4:
            return False

    return True


@profile
def createMandelbrotFractal():
    """
    This function will create a png file of a mandelbrot fractal
    """

    t = time.clock()

    # Creates Z0
    startVal = np.zeros((HEIGHT, WIDTH), dtype=np.complex128)

    # Creates the constant array C in Zn = Zn-1*Zn-1 + C
    c = np.mgrid[0:HEIGHT, 0:WIDTH]

    # Generates the fractal
    result = mandelbrot(startVal, c[0], c[1])

    t = np.floor((time.clock()-t)*1000)
    print("Generated mandelbrot fractal in {} ms".format(t))

    # Save the fractal to a file
    im = Image.fromarray(np.uint8(result*255), mode="L")
    im.save('mandelbrot.png')


if __name__ == '__main__':
    createMandelbrotFractal()
