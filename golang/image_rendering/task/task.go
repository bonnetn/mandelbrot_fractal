package task

import "mandelbrot/image_rendering"

// Task is fed to the workers to process all the positions until the task is finished
type Task interface {
	IsFinished() bool
	GetNextPosition() *image_rendering.Position
}