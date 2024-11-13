package pkg

import (
	"crypto/rand"
	"encoding/hex"
	"fmt"
)

func GenerateRandomID(length int) (string, error) {
	bytes := make([]byte, length)
	if _, err := rand.Read(bytes); err != nil {
		return "", err
	}
	return hex.EncodeToString(bytes), nil
}

func Ternary(condition bool, trueVal, falseVal any) any {
	if condition {
		return trueVal
	}
	return falseVal
}

func AnyToString(val interface{}) string {
	return fmt.Sprintf("%v", val)
}
