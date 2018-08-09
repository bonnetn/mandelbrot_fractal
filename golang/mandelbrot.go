package main

import (
	"fmt"
	"image"
	"image/png"
	"math"
	"os"
	"runtime"
	"time"
)

const (
	iterationCount int = 30
	width          int = 1366
	height         int = 768
	chunkSize      int = 512
)

func main() {

	start := time.Now()

	// fan-out
	taskChan := make(chan task, 8)
	go func() {
		for i := 0; i < int(math.Ceil(float64(width*height)/float64(chunkSize))); i++ {
			end := (i + 1) * chunkSize
			if end >= width*height {
				end = width * height
			}
			taskChan <- &chunkOfPixelsTask{i * chunkSize, end, width}
		}
		close(taskChan)
	}()

	// workers
	fmt.Printf("Starting %d workers\n", runtime.NumCPU())
	resultChan := make(chan *pixel, 8)
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
		img.Pix[p.pos.y*img.Stride+p.pos.x] = p.value
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

type task interface {
	isFinished() bool
	getNextPosition() *position
}

type chunkOfPixelsTask struct {
	i        int
	end      int
	imgWidth int
}

func (t *chunkOfPixelsTask) isFinished() bool {
	return t.i == t.end
}

func (t *chunkOfPixelsTask) getNextPosition() (pos *position) {
	pos = &position{
		t.i % t.imgWidth,
		t.i / t.imgWidth,
	}
	t.i++
	return
}

func worker(resultChan chan *pixel, taskChan chan task) {
	for {
		task, ok := <-taskChan
		if !ok {
			return
		}

		for !task.isFinished() {
			pos := task.getNextPosition()

			var v uint8 = 0
			if mandelbrot(pos.x, pos.y) {
				v = 255
			}

			resultChan <- &pixel{v, *pos}
		}
	}
}

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

type pixel struct {
	value uint8
	pos   position
}

type position struct {
	x int
	y int
}
