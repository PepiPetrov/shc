package internal

import (
	"encoding/binary"
	"errors"
)

// Package represents a data package with TaskID, buffer, and length.
type Package struct {
	TaskID string
	Buffer []byte
	Length uint32
}

func BufferAddUint32(buf []byte, num uint32) []byte {
	var integer32 = make([]byte, 4)
	binary.LittleEndian.PutUint32(integer32, num)
	buf = append(buf, integer32...)
	return buf
}

func BufferAddBytes(buf []byte, bytes []byte) []byte {
	buf = BufferAddUint32(buf, uint32(len(bytes)))
	buf = append(buf, bytes...)
	return buf
}

func BufferAddString(buf []byte, str string) []byte {
	return BufferAddBytes(buf, []byte(str))
}

// PackageReader helps in reading data from the package buffer.
type PackageReader struct {
	Buffer []byte
	Pos    int
}

// NewPackageReader creates a new PackageReader for the given package.
func NewPackageReader(pkg *Package) *PackageReader {
	return &PackageReader{
		Buffer: pkg.Buffer,
		Pos:    0,
	}
}

// GetUint32 reads a uint32 value from the package buffer.
func (pr *PackageReader) GetUint32() (uint32, error) {
	if len(pr.Buffer[pr.Pos:]) < 4 {
		return 0, errors.New("not enough data to read uint32")
	}
	value := binary.BigEndian.Uint32(pr.Buffer[pr.Pos : pr.Pos+4])
	pr.Pos += 4
	return value, nil
}

// GetBytes reads a length-prefixed byte array from the package buffer.
func (pr *PackageReader) GetBytes() ([]byte, error) {
	length, err := pr.GetUint32()
	if err != nil {
		return nil, err
	}
	if len(pr.Buffer[pr.Pos:]) < int(length) {
		return nil, errors.New("not enough data to read bytes")
	}
	data := pr.Buffer[pr.Pos : pr.Pos+int(length)]
	pr.Pos += int(length)
	return data, nil
}

// GetString reads a length-prefixed string from the package buffer.
func (pr *PackageReader) GetString() (string, error) {
	data, err := pr.GetBytes()
	if err != nil {
		return "", err
	}
	return string(data), nil
}
