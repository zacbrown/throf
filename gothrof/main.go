package main

import (
	"bytes"
	"container/list"
	"fmt"
	"io/ioutil"
	"os"
	"strings"
)

func main() {
	if len(os.Args) == 1 {
		usage()
		return
	}

	file := os.Args[1]
	dat, _ := getFileAsString(file)

	interpreter := &Interpreter{}
	interpreter.Init(tokenize(dat))

	return
}

func usage() {
	fmt.Println("Usage: ./gothrof <code file>.th4")
}

func getFileAsString(file string) (string, error) {
	data, err := ioutil.ReadFile(file)
	return string(data), err
}

func tokenize(input string) list.List {
	reader := strings.NewReader(input)
	var buffer bytes.Buffer
	var tokens list.List

	for ch, _, err := reader.ReadRune(); err == nil; ch, _, err = reader.ReadRune() {
		if ch == '\t' || ch == ' ' || ch == '\n' || ch == '\r' || ch == '\f' {
			if buffer.Len() > 0 {
				tokens.PushBack(buffer.String())
				buffer.Reset()
			}
		} else {
			buffer.WriteRune(ch)
		}
	}

	return tokens
}
