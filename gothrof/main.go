package main

import (
	"fmt"
	"os"
)

func main() {
	if len(os.Args) == 1 {
		usage()
		return
	}

	file := os.Args[1]

	interpreter := &Interpreter{}
	interpreter.Init()

	interpreter.Execute(file)

	return
}

func usage() {
	fmt.Println("Usage: ./gothrof <code file>.th4")
}
