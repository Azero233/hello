// hello_test.go
package hello_test

import (
	"fmt"
	"testing"

	"github.com/Azero233/hello"
)

func TestHello(t *testing.T) {
	data := "jack"
	expected := fmt.Sprintf("hello %s!\n", data)
	result := hello.Hello(data)

	if result != expected {
		t.Fatalf("expected result %s, but got %s", expected, result)
	}
}
