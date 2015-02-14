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

func validateDepth(t *testing.T, expectedDepth int, actualDepth int) {
	if expectedDepth != actualDepth {
		t.Fatalf("Expected stack depth of %d", actualDepth)
	}
}

func TestDrop(t *testing.T) {
	toks := tokenize("2 2 drop")
	interpreter.Init(toks)
	interpreter.Execute()

	dstack := interpreter.GetDStack()
	validateDepth(t, dstack.Length(), 1)

	if dstack.Peek() != "2" {
		t.Error("Expected top element to be '2'")
	}
}

func TestSwap(t *testing.T) {
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
}

func TestDup(t *testing.T) {
	toks := tokenize("2 dup")
	interpreter.Init(toks)

	// step one to make sure 2 is pushed
	interpreter.Step()
	dstack := interpreter.GetDStack()

	top := dstack.Peek()
	if top != "2" {
		t.Fatalf("Expected '2' to be on top of dstack, got %d", top)
	}

	interpreter.Execute()

	validateDepth(t, dstack.Length(), 2)

	newTop := dstack.Pop()
	next := dstack.Pop()

	if top != next {
		t.Errorf("Expected the two elements on the dstack to be equal. top: %d, next: %d", top, next)
	}

	if newTop != top {
		t.Errorf("Expected two elements of same value on dstack. expected: %d, got: %d", top, newTop)
	}
}

func TestOver(t *testing.T) {
	toks := tokenize("1 2 over")
	interpreter.Init(toks)
	interpreter.Execute()

	dstack := interpreter.GetDStack()

	validateDepth(t, dstack.Length(), 3)

	first := dstack.Pop()
	second := dstack.Pop()
	third := dstack.Pop()

	if first != "1" || second != "2" || third != "1" {
		t.Fatalf("Expected order of 1, 2, 1 for stack, got %s, %s, %s", first, second, third)
	}
}

func TestRot(t *testing.T) {
	toks := tokenize("1 2 3 rot")
	interpreter.Init(toks)
	interpreter.Execute()

	dstack := interpreter.GetDStack()

	validateDepth(t, dstack.Length(), 3)

	first := dstack.Pop()
	second := dstack.Pop()
	third := dstack.Pop()

	if first != "1" || second != "3" || third != "2" {
		t.Fatalf("Expected order of 2, 3, 1 for stack, got %s, %s, %s", first, second, third)
	}
}

func TestNRot(t *testing.T) {
	toks := tokenize("1 2 3 -rot")
	interpreter.Init(toks)
	interpreter.Execute()

	dstack := interpreter.GetDStack()

	validateDepth(t, dstack.Length(), 3)

	first := dstack.Pop()
	second := dstack.Pop()
	third := dstack.Pop()

	if first != "2" || second != "1" || third != "3" {
		t.Fatalf("Expected order of 2, 1, 3 for stack, got %s, %s, %s", first, second, third)
	}
}

func Test2Drop(t *testing.T) {
	toks := tokenize("1 2 2drop")
	interpreter.Init(toks)
	interpreter.Execute()

	dstack := interpreter.GetDStack()

	validateDepth(t, dstack.Length(), 0)
}

func Test2Dup(t *testing.T) {
	toks := tokenize("1 2 2dup")
	interpreter.Init(toks)
	interpreter.Execute()

	dstack := interpreter.GetDStack()

	validateDepth(t, dstack.Length(), 4)

	first := dstack.Pop()
	second := dstack.Pop()
	third := dstack.Pop()
	fourth := dstack.Pop()

	if first != "2" || second != "1" || third != "2" || fourth != "1" {
		t.Fatalf("Expected 1, 2, 1, 2 for stack, got %s, %s, %s, %s", fourth, third, second, first)
	}
}
