package main

import (
	"os"
	"testing"
)

var interpreter *Interpreter

func TestMain(m *testing.M) {
	interpreter = &Interpreter{}
	os.Exit(m.Run())

}

func TestDrop(t *testing.T) {
	t.Log("Testing 'drop'")

	toks := tokenize("2 2 drop")
	interpreter.Init(toks)
	interpreter.Execute()

	if interpreter.GetDStack().Length() != 1 {
		t.Error("Expected stack depth of 1")
	}
	if interpreter.GetDStack().Peek() != "2" {
		t.Error("Expected top element to be '2'")
	}
}

func TestSwap(t *testing.T) {
	t.Log("Testing 'swap'")

	toks := tokenize("2 3 swap")
	interpreter.Init(toks)
	interpreter.Execute()

	dstack := interpreter.GetDStack()
	elem := dstack.Pop()
	if elem != "2" {
		t.Errorf("Expected element '2', got '%v'", elem)
	}

	elem = dstack.Pop()
	if elem != "3" {
		t.Errorf("Expected element '3', got '%v'", elem)
	}

	if interpreter.GetDStack().Length() != 0 {
		t.Errorf("Expected stack depth of 0, got %d", interpreter.GetDStack().Length())
	}
}

func TestDup(t *testing.T) {
	t.Log("Testing 'dup'")

	toks := tokenize("2 dup")
	interpreter.Init(toks)

	// step one to make sure 2 is pushed
	interpreter.Step()
	dstack := interpreter.GetDStack()

	top := dstack.Peek()
	if top != "2" {
		t.Errorf("Expected '2' to be on top of dstack, got %d", top)
	}

	interpreter.Execute()

	if dstack.Length() != 2 {
		t.Errorf("Expected two elements on the dstack, got depth %d", dstack.Length())
	}

	newTop := dstack.Pop()
	next := dstack.Pop()

	if top != next {
		t.Errorf("Expected the two elements on the dstack to be equal. top: %d, next: %d", top, next)
	}

	if newTop != top {
		t.Errorf("Expected two elements of same value on dstack. expected: %d, got: %d", top, newTop)
	}
}
