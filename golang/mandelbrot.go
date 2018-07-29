package main

import (
	"os"
	"image/png"
	"time"
	"fmt"
	"image"
	"runtime"
)

const (
	iterationCount int = 30
	width          int = 1366
	height         int = 768
)

func mandelbrot(x, y int) bool {
	const (
		half_width  float64 = float64(width) / 2
		half_height float64 = float64(height) / 2
	)

	a := complex(0, 0)
	c := complex((float64(x)-half_width)/half_height, (float64(y)-half_height)/half_height)

	for i := 0; i < iterationCount; i++ {
		a = a*a + c
		
		if real(a)*real(a)+imag(a)*imag(a) >= 4 {
			return false
		}
	}

	return true

}

type pixel struct {
	value uint8
	x     int
	y     int
}

type lineTask struct {
	line int
}

func worker(resultChan chan pixel, taskChan chan lineTask) {
	for {
		task, ok := <-taskChan
		if !ok {
			return
		}
		line := task.line

		for y := 0; y < height; y++ {
			var v uint8 = 0
			if mandelbrot(line, y) {
				v = 255
			}
			resultChan <- pixel{v, line, y}
		}
	}
}

func main() {

	start := time.Now()

	// fan-out
	taskChan := make(chan lineTask, width)
	go func() {
		for x := 0; x < width; x++ {
			taskChan <- lineTask{x}
		}
		close(taskChan)
	}()

	// workers
	resultChan := make(chan pixel, width)
	defer close(resultChan)
	go func() {
		for i := 0; i < runtime.NumCPU(); i++ {
			go worker(resultChan, taskChan)
		}
	}()

	// drawer
	img := image.NewGray(image.Rect(0, 0, width, height))
	for i := 0; i < width*height; i++ {
		p := <-resultChan
		img.Pix[p.y*img.Stride+p.x] = p.value
	}

	elapsed := time.Since(start)
	fmt.Printf("%s", elapsed)

	outputFile, err := os.Create("mandelbrot.png")
	defer outputFile.Close()
	if err != nil {
		panic(err)
	}

	png.Encode(outputFile, img)
}
