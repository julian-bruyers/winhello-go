package main

import (
	"fmt"
	"log"
	"winhello"
)

func main() {
	fmt.Println("Windows Hello Authentication Test")

	isAuthenticated, err := winhello.Authenticate("Verify your identity for winhello-go test")

	if err != nil {
		log.Fatal(err)
	}

	if isAuthenticated {
		fmt.Println("Authentication successful!")
	} else {
		fmt.Println("Authentication failed!")
	}
}
