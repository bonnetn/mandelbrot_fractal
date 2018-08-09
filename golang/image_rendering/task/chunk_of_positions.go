package task

import "mandelbrot/image_rendering"

// ChunkOfPositions is a task of positions that will be processed sequentially
type ChunkOfPositions struct {
	i        int
	end      int
	imgWidth int
}

func (t *ChunkOfPositions) IsFinished() bool {
	return t.i == t.end
}

func (t *ChunkOfPositions) GetNextPosition() (pos *image_rendering.Position) {
	pos = &image_rendering.Position{
		t.i % t.imgWidth,
		t.i / t.imgWidth,
	}
	t.i++
	return
}

func NewChunkOfPixelsTask(start, end, width int) *ChunkOfPositions {
	return &ChunkOfPositions{start, end, width}
}
