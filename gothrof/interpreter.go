package main

import (
	"container/list"
	"fmt"
)

const (
	OP_DROP      = iota
	OP_SWAP      = iota
	OP_DUP       = iota
	OP_OVER      = iota
	OP_ROT       = iota
	OP_NROT      = iota
	OP_2DROP     = iota
	OP_2DUP      = iota
	OP_QDUP      = iota // ?dup
	OP_INCR      = iota
	OP_DECR      = iota
	OP_ADD       = iota
	OP_SUB       = iota
	OP_MUL       = iota
	OP_DIVMOD    = iota
	OP_EQU       = iota
	OP_NEQU      = iota
	OP_LT        = iota
	OP_GT        = iota
	OP_LE        = iota
	OP_GE        = iota
	OP_ZEQU      = iota
	OP_ZNEQU     = iota
	OP_ZLT       = iota
	OP_ZGT       = iota
	OP_ZLE       = iota
	OP_ZGE       = iota
	OP_AND       = iota
	OP_OR        = iota
	OP_XOR       = iota
	OP_CREATE    = iota
	OP_COMMA     = iota
	OP_LBRAC     = iota // switch to immediate mode, immediate word
	OP_RBRAC     = iota // switch to compile mode
	OP_COLON     = iota
	OP_SEMICOLON = iota // immediate word
	OP_BRANCH    = iota
	OP_ZBRANCH   = iota
	OP_IMMEDIATE = iota
)

type Word struct {
	name       string
	definition func(interpreter *Interpreter)
}

type Interpreter struct {
	dstack    Stack     // data stack
	rstack    Stack     // return stack
	stream    list.List // token stream
	latest    list.List // dictionary head
	base      int       // current base for printing/reading numbers
	immediate bool      // signal for whether IMMEDIATE mode is on
}

func (i *Interpreter) Init(tokens list.List) {
	i.stream = tokens
	i.dstack = Stack{}
	i.rstack = Stack{}

	i.addWordToDictionary("drop", func(inter *Interpreter) { inter.dpop() })
	i.addWordToDictionary("swap", func(inter *Interpreter) {
		top := inter.dpop()
		next := inter.dpop()
		inter.dpush(top)
		inter.dpush(next)
	})
	i.addWordToDictionary("dup", func(inter *Interpreter) {
		inter.dpush(inter.dpeek())
	})
	i.addWordToDictionary("over", func(inter *Interpreter) {
		inter.dstack.InsertAfter(1, inter.dpeek())
	})
	i.addWordToDictionary("rot", func(inter *Interpreter) {
		top := inter.dpop()
		inter.dstack.InsertAfter(2, top)
	})
}

func (i *Interpreter) DumpStack() {
	fmt.Println("Stack")
	fmt.Println("==============")
	for cur := i.dstack.top; cur != nil; cur = cur.next {
		fmt.Printf("%v\n", cur)
	}
}

func (i *Interpreter) GetDStack() *Stack {
	return &i.dstack
}

func (i *Interpreter) GetRStack() *Stack {
	return &i.rstack
}

func (i *Interpreter) Execute() {
	for current := i.stream.Front(); current != nil; current = current.Next() {
		word := i.findWordInDictionary(current.Value.(string))
		if word == nil {
			i.dpush(current.Value.(string))
		} else {
			word.definition(i)
		}
	}
}

func (i *Interpreter) addWordToDictionary(name string,
	definition func(interpreter *Interpreter)) {

	i.latest.PushFront(&Word{name, definition})
}

func (i *Interpreter) findWordInDictionary(name string) *Word {
	for front := i.latest.Front(); front != nil; front = front.Next() {
		if front.Value.(*Word).name == name {
			return front.Value.(*Word)
		}
	}

	return nil
}

func (i *Interpreter) rpush(val interface{}) {
	i.rstack.Push(val)
}

func (i *Interpreter) rpop() interface{} {
	return i.rstack.Pop()
}

func (i *Interpreter) dpush(val interface{}) {
	i.dstack.Push(val)
}

func (i *Interpreter) dpop() interface{} {
	return i.dstack.Pop()
}

func (i *Interpreter) dpeek() interface{} {
	return i.dstack.Peek()
}
