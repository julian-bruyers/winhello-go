package main

import (
	"fmt"

	"winhello"
)

func main() {
	fmt.Println("Windows Hello Authentication Test")
	ok, _ := winhello.Authenticate("Verify your identity for winhello-go test")
	fmt.Print(ok)
}
