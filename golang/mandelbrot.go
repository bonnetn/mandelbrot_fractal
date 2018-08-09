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
)

const (
	iterationCount int = 30
	width          int = 1366
	height         int = 768
	chunkSize      int = 512
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

	// Make the image
	img := image.NewGray(image.Rect(0, 0, width, height))

	// workers that will process the tasks and actually do the rendering in parallel
	workersNum := runtime.NumCPU()
	fmt.Printf("Starting %d workers\n", workersNum)
	stopChan := make(chan bool)
	defer close(stopChan)
	go func() {
		for i := 0; i < runtime.NumCPU(); i++ {
			go worker(img, taskChan, stopChan)
		}
	}()

	// wait for all workers to stop
	for i := 0; i < workersNum; i++ {
		<-stopChan
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
func worker(result *image.Gray, taskChan chan task.Task, stopChan chan bool) {
	for {
		t, ok := <-taskChan
		if !ok {
			stopChan <- true
			return
		}

		for !t.IsFinished() {
			pos := t.GetNextPosition()

			var v uint8 = 0
			if mandelbrot(pos.X, pos.Y) {
				v = 255
			}

			result.Pix[pos.Y*result.Stride+pos.X] = v
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
