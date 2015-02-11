package main

import (
	"container/list"
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
)

type Interpreter struct {
	dstack    Stack     // data stack
	rstack    Stack     // return stack
	stream    list.List // token stream
	latest    list.List // dictionary head
	base      int       // current base for printing/reading numbers
	immediate bool      // signal for whether IMMEDIATE mode is on
}

func (i *Interpreter) Init(tokens list.List) {

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
