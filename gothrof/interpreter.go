package main

import (
	"container/list"
	"fmt"
	"strconv"
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

type CodeWord func(*Interpreter)

type Word struct {
	name       string
	immediate  bool
	definition *list.List
}

type Interpreter struct {
	dstack *Stack     // data stack
	rstack *Stack     // return stack
	stream *list.List // token stream
	latest list.List  // dictionary head
	base   int        // current base for printing/reading numbers
	state  bool       // 'false' == immediate, 'true' == compiling
}

func (i *Interpreter) Init(tokens *list.List) {
	i.stream = tokens
	i.dstack = &Stack{}
	i.rstack = &Stack{}

	i.addNormalPrimitiveToDictionary("true", func(inter *Interpreter) { inter.dpush(true) })
	i.addNormalPrimitiveToDictionary("false", func(inter *Interpreter) { inter.dpush(false) })
	i.addNormalPrimitiveToDictionary("drop", func(inter *Interpreter) { inter.dpop() })
	i.addNormalPrimitiveToDictionary("swap", func(inter *Interpreter) {
		top := inter.dpop()
		next := inter.dpop()
		inter.dpush(top)
		inter.dpush(next)
	})
	i.addNormalPrimitiveToDictionary("dup", func(inter *Interpreter) {
		inter.dpush(inter.dpeek())
	})
	i.addNormalPrimitiveToDictionary("over", func(inter *Interpreter) {
		elem := inter.dstack.GetAt(1)
		inter.dpush(elem)
	})
	i.addNormalPrimitiveToDictionary("rot", func(inter *Interpreter) {
		elem := inter.dstack.RemoveAt(2)
		inter.dpush(elem)
	})
	i.addNormalPrimitiveToDictionary("-rot", func(inter *Interpreter) {
		elem := inter.dpop()
		inter.dstack.InsertAfter(1, elem)
	})
	i.addNormalPrimitiveToDictionary("2drop", func(inter *Interpreter) {
		inter.dpop()
		inter.dpop()
	})
	i.addNormalPrimitiveToDictionary("2dup", func(inter *Interpreter) {
		elem1 := inter.dstack.GetAt(1)
		elem2 := inter.dpeek()
		inter.dpush(elem1)
		inter.dpush(elem2)
	})
	i.addNormalPrimitiveToDictionary("?dup", func(inter *Interpreter) {
		elem := inter.dpeek()
		if elem.(Number).val != 0 {
			inter.dpush(elem)
		}
	})
	i.addNormalPrimitiveToDictionary("+", func(inter *Interpreter) {
		rhs := inter.dpop()
		lhs := inter.dpop()
		inter.dpush(lhs.(Number).Add(rhs.(Number)))
	})
	i.addNormalPrimitiveToDictionary("-", func(inter *Interpreter) {
		rhs := inter.dpop()
		lhs := inter.dpop()
		inter.dpush(lhs.(Number).Sub(rhs.(Number)))
	})
	i.addNormalPrimitiveToDictionary("*", func(inter *Interpreter) {
		rhs := inter.dpop()
		lhs := inter.dpop()
		inter.dpush(lhs.(Number).Mul(rhs.(Number)))
	})
	i.addNormalPrimitiveToDictionary("/", func(inter *Interpreter) {
		rhs := inter.dpop()
		lhs := inter.dpop()
		inter.dpush(lhs.(Number).Div(rhs.(Number)))
	})
	i.addNormalPrimitiveToDictionary("incr", func(inter *Interpreter) {
		elem := inter.dpop()
		inter.dpush(elem.(Number).Add(NewInt(1)))
	})
	i.addNormalPrimitiveToDictionary("decr", func(inter *Interpreter) {
		elem := inter.dpop()
		inter.dpush(elem.(Number).Sub(NewInt(1)))
	})
	i.addNormalPrimitiveToDictionary("mod", func(inter *Interpreter) {
		rhs := inter.dpop()
		lhs := inter.dpop()
		inter.dpush(lhs.(Number).Mod(rhs.(Number)))
	})
	i.addNormalPrimitiveToDictionary("=", func(inter *Interpreter) {
		rhs := inter.dpop()
		lhs := inter.dpop()
		inter.dpush(lhs.(Number).Equals(rhs.(Number)))
	})
	i.addNormalPrimitiveToDictionary("<>", func(inter *Interpreter) {
		rhs := inter.dpop()
		lhs := inter.dpop()
		inter.dpush(!lhs.(Number).Equals(rhs.(Number)))
	})
	i.addNormalPrimitiveToDictionary("<", func(inter *Interpreter) {
		rhs := inter.dpop()
		lhs := inter.dpop()
		inter.dpush(lhs.(Number).LessThan(rhs.(Number)))
	})
	i.addNormalPrimitiveToDictionary("<=", func(inter *Interpreter) {
		rhs := inter.dpop().(Number)
		lhs := inter.dpop().(Number)
		inter.dpush(lhs.LessThan(rhs) || lhs.Equals(rhs))
	})
	i.addNormalPrimitiveToDictionary(">", func(inter *Interpreter) {
		rhs := inter.dpop().(Number)
		lhs := inter.dpop().(Number)
		inter.dpush(!lhs.LessThan(rhs) && !lhs.Equals(rhs))
	})
	i.addNormalPrimitiveToDictionary(">=", func(inter *Interpreter) {
		rhs := inter.dpop().(Number)
		lhs := inter.dpop().(Number)
		inter.dpush(!lhs.LessThan(rhs))
	})
	i.addNormalPrimitiveToDictionary("and", func(inter *Interpreter) {
		rhs := inter.dpop().(bool)
		lhs := inter.dpop().(bool)
		inter.dpush(lhs && rhs)
	})
	i.addNormalPrimitiveToDictionary("or", func(inter *Interpreter) {
		rhs := inter.dpop().(bool)
		lhs := inter.dpop().(bool)
		inter.dpush(lhs || rhs)
	})
	i.addNormalPrimitiveToDictionary("xor", func(inter *Interpreter) {
		rhs := inter.dpop().(bool)
		lhs := inter.dpop().(bool)
		inter.dpush((!rhs && lhs) || (rhs && !lhs))
	})
	i.addImmediatePrimitiveToDictionary("[", func(inter *Interpreter) {
		inter.state = false
	})
	i.addNormalPrimitiveToDictionary("]", func(inter *Interpreter) {
		inter.state = true
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
	return i.dstack
}

func (i *Interpreter) GetRStack() *Stack {
	return i.rstack
}

// this should only return 'int', 'double' or 'Rational'
func parseNumeral(content string) (Number, error) {

	parsedInt, err := strconv.Atoi(content)
	if err == nil {
		return NewInt(parsedInt), nil
	}

	parsedFloat, err := strconv.ParseFloat(content, 64)
	if err == nil {
		return NewFloat(parsedFloat), nil
	}
	/*
		parsedRational, err := ParseRational(content)
		if err == nil {
			return parsedRational
		}
	*/
	return Number{}, fmt.Errorf("Invalid numeric format: '%s'", content)
}

func (i *Interpreter) Step() bool {
	current := i.stream.Front()

	if current == nil {
		return false
	}

	word := i.findWordInDictionary(current.Value.(string))
	if word == nil {
		dataAsString := current.Value.(string)

		parsedNum, err := parseNumeral(dataAsString)

		if err == nil {
			i.dpush(parsedNum)
		} else {
			// just push it on as a string/random thing otherwise
			i.dpush(dataAsString)
		}
	} else {
		for codeword := word.definition.Front(); codeword != nil; codeword = codeword.Next() {
			codeword.Value.(CodeWord)(i)
		}
	}

	i.stream.Remove(current)
	return true
}

func (i *Interpreter) Execute() {
	for i.Step() == true {
	}
}

func (i *Interpreter) addPrimitiveWordToDictionary(name string, immediate bool,
	definition CodeWord) {

	word := &Word{}
	word.name = name
	word.immediate = immediate
	word.definition = &list.List{}
	word.definition.PushBack(definition)

	i.latest.PushFront(word)
}

func (i *Interpreter) addImmediatePrimitiveToDictionary(name string, definition CodeWord) {
	i.addPrimitiveWordToDictionary(name, false, definition)
}

func (i *Interpreter) addNormalPrimitiveToDictionary(name string, definition CodeWord) {
	i.addPrimitiveWordToDictionary(name, true, definition)
}

func (i *Interpreter) addWordToDictionary(name string, immediate bool,
	definition *list.List) {

	i.latest.PushFront(&Word{name, immediate, definition})
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
