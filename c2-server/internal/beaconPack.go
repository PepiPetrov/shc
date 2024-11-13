package internal

import (
	"bytes"
	"encoding/binary"
	"unicode/utf16"
)

type BeaconPack struct {
	buffer bytes.Buffer
	size   uint32
}

// GetBuffer returns the buffer with the total size packed at the beginning.
// The buffer is prefixed with a 4-byte integer representing the total size of the data.
func (b *BeaconPack) GetBuffer() ([]byte, error) {
	buf := new(bytes.Buffer)
	// Write the size in little-endian format
	if err := binary.Write(buf, binary.LittleEndian, b.size); err != nil {
		return nil, err
	}
	// Write the actual buffer content
	buf.Write(b.buffer.Bytes())
	return buf.Bytes(), nil
}

// AddShort adds a 2-byte short value to the buffer.
func (b *BeaconPack) AddShort(value int16) error {
	if err := binary.Write(&b.buffer, binary.LittleEndian, value); err != nil {
		return err
	}
	b.size += 2
	return nil
}

// AddInt adds a 4-byte integer to the buffer.
func (b *BeaconPack) AddInt(value int32) error {
	if err := binary.Write(&b.buffer, binary.LittleEndian, value); err != nil {
		return err
	}
	b.size += 4
	return nil
}

// AddStr adds a UTF-8 string to the buffer, prefixed with its length and null-terminated.
func (b *BeaconPack) AddStr(s string) error {
	sBytes := []byte(s)
	length := uint32(len(sBytes) + 1) // +1 for the null terminator
	if err := binary.Write(&b.buffer, binary.LittleEndian, length); err != nil {
		return err
	}
	if _, err := b.buffer.Write(sBytes); err != nil {
		return err
	}
	b.buffer.WriteByte(0) // Null terminator
	b.size += 4 + length
	return nil
}

// AddWStr adds a UTF-16 wide string to the buffer, prefixed with its length and null-terminated.
func (b *BeaconPack) AddWStr(s string) error {
	// Convert string to UTF-16
	sWide := utf16.Encode([]rune(s))
	length := uint32(len(sWide)*2 + 2) // +2 for the null terminator (UTF-16 uses 2 bytes)
	if err := binary.Write(&b.buffer, binary.LittleEndian, length); err != nil {
		return err
	}
	// Write UTF-16 encoded string
	for _, wchar := range sWide {
		if err := binary.Write(&b.buffer, binary.LittleEndian, wchar); err != nil {
			return err
		}
	}
	// Write UTF-16 null terminator
	if err := binary.Write(&b.buffer, binary.LittleEndian, uint16(0)); err != nil {
		return err
	}
	b.size += 4 + length
	return nil
}

// AddBin adds a binary data block to the buffer, prefixed with its length.
func (b *BeaconPack) AddBin(data []byte) error {
	length := uint32(len(data))
	if err := binary.Write(&b.buffer, binary.LittleEndian, length); err != nil {
		return err
	}
	if _, err := b.buffer.Write(data); err != nil {
		return err
	}
	b.size += 4 + length
	return nil
}

// Reset clears the buffer and resets the size to 0.
func (b *BeaconPack) Reset() {
	b.buffer.Reset()
	b.size = 0
}
