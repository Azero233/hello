// hello.go
package hello

import "fmt"

func Hello(name string) string {
	if name == "" {
		name = "world"
	}
	return fmt.Sprintf("hello %s!\n", name)
}
