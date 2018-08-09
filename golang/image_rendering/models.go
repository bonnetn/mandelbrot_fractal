package image_rendering

type Pixel struct {
	Value uint8
	Pos   Position
}

type Position struct {
	X int
	Y int
}