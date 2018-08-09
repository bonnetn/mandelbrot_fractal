package main

import (
	"fmt"
	"image"
	"image/png"
	"math"
	"os"
	"runtime"
	"time"
	"mandelbrot/image_rendering/task"
	"mandelbrot/image_rendering"
)

const (
	iterationCount    int = 30
	width             int = 1366
	height            int = 768
	chunkSize         int = 512
	resultChannelSize int = 16
)

func main() {

	start := time.Now()

	// make a queue of tasks (chunks of pixels that need to be rendered)
	taskChan := make(chan task.Task, 16)
	go func() {
		for i := 0; i < int(math.Ceil(float64(width*height)/float64(chunkSize))); i++ {
			end := (i + 1) * chunkSize
			if end >= width*height {
				end = width * height
			}
			taskChan <- task.NewChunkOfPixelsTask(i*chunkSize, end, width)
		}
		close(taskChan)
	}()

	// workers that will process the tasks and actually do the rendering in parallel
	fmt.Printf("Starting %d workers\n", runtime.NumCPU())
	resultChan := make(chan *image_rendering.Pixel, resultChannelSize)
	defer close(resultChan)
	go func() {
		for i := 0; i < runtime.NumCPU(); i++ {
			go worker(resultChan, taskChan)
		}
	}()

	// write the results of the workers in an image array
	img := image.NewGray(image.Rect(0, 0, width, height))
	for i := 0; i < width*height; i++ {
		p := <-resultChan
		img.Pix[p.Pos.Y*img.Stride+p.Pos.X] = p.Value
	}

	elapsed := time.Since(start)
	fmt.Printf("%s\n", elapsed)

	outputFile, err := os.Create("mandelbrot.png")
	defer outputFile.Close()
	if err != nil {
		panic(err)
	}

	png.Encode(outputFile, img)
}

// a worker dequeue tasks, render the pixels and enqueue the rendered pixel in resultChan
func worker(resultChan chan *image_rendering.Pixel, taskChan chan task.Task) {
	for {
		t, ok := <-taskChan
		if !ok {
			return
		}

		for !t.IsFinished() {
			pos := t.GetNextPosition()

			var v uint8 = 0
			if mandelbrot(pos.X, pos.Y) {
				v = 255
			}

			resultChan <- &image_rendering.Pixel{v, *pos}
		}
	}
}

// mandelbrot computes mandelbrot fractal at a given position
func mandelbrot(x, y int) bool {
	const (
		halfWidth  = float64(width) / 2
		halfHeight = float64(height) / 2
	)

	a := complex(0, 0)
	c := complex((float64(x)-halfWidth)/halfHeight*2, (float64(y)-halfHeight)/halfHeight*2)

	for i := 0; i < iterationCount; i++ {
		//a = a*a + c
		a *= a
		a += c

		if real(a)*real(a)+imag(a)*imag(a) >= 4 {
			return false
		}
	}

	return true
}
