Mandelbrot fractal generator
============================

This script creates a PNG file.

Python
------

It is just a quick project to get a hand on the **numba** library. 

The generation of the fractal is parallelized and can even use *CUDA* (by
changing the target).


C++
---
I decided to compare the performance of *numba* and native C++. So I made a 
small C++ code to see the speed difference, using multithreading. 
(unfortunately I cannot try CUDA)


Golang
---
When I discovered Go language I wanted to practice a bit so I re-coded this
script with Go. I also wanted to see how it performed compared to C++ and 
Python.

