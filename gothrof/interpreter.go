package main

import (
	"bytes"
	"container/list"
	"fmt"
	"io/ioutil"
	"strconv"
	"strings"
)

type CodeWord func(*Interpreter)
type StringLiteral struct {
	string
}
type Quotation struct {
	list.List
}

type Word struct {
	name       string
	immediate  bool
	definition *Quotation
}

type Invokable interface {
	Invoke(i *Interpreter)
}

func (q *Quotation) Invoke(i *Interpreter) {
	for codeword := q.Front(); codeword != nil; codeword = codeword.Next() {
		switch codeword.Value.(type) {
		case CodeWord:
			codeword.Value.(CodeWord)(i)
		case *Word:
			codeword.Value.(*Word).definition.Invoke(i)
		case bool:
			i.dpush(codeword.Value.(bool))
		case Number:
			i.dpush(codeword.Value.(Number))
		case StringLiteral:
			i.dpush(codeword.Value.(StringLiteral))
		}
	}
}

type Interpreter struct {
	dstack *Stack     // data stack
	rstack *Stack     // return stack
	stream *list.List // token stream
	latest list.List  // dictionary head
	base   int        // current base for printing/reading numbers
	state  bool       // 'false' == immediate, 'true' == compiling
}

func collectStringLiteral(r *strings.Reader) StringLiteral {
	// pick up the opening quotation mark
	for ch, _, err := r.ReadRune(); err == nil && ch != '"'; ch, _, err = r.ReadRune() {

	}

	var buffer bytes.Buffer

	// collect till we get the closing quotation mark
	for ch, _, err := r.ReadRune(); err == nil && ch != '"'; ch, _, err = r.ReadRune() {
		buffer.WriteRune(ch)
	}

	return StringLiteral{buffer.String()}
}

func collectQuotation(r *strings.Reader) *Quotation {
	var buffer bytes.Buffer
	for ch, _, err := r.ReadRune(); err == nil && ch != ']'; ch, _, err = r.ReadRune() {
		buffer.WriteRune(ch)
	}

	parsedTokens := tokenize(buffer.String())

	return &Quotation{*parsedTokens}
}

func tokenize(input string) *list.List {
	reader := strings.NewReader(input)
	var buffer bytes.Buffer
	tokens := &list.List{}

	pushElem := func() {
		token := buffer.String()
		buffer.Reset()

		tokenNumeral, err := parseNumeral(token)
		if err == nil {
			tokens.PushBack(tokenNumeral)
		} else {
			tokens.PushBack(token)
		}
	}

	for ch, _, err := reader.ReadRune(); err == nil; ch, _, err = reader.ReadRune() {

		if buffer.String() == "string" {

			buffer.Reset()
			tokens.PushBack(collectStringLiteral(reader))
		} else if ch == '[' {

			tokens.PushBack(collectQuotation(reader))
		} else if ch == '\t' || ch == ' ' || ch == '\n' || ch == '\r' || ch == '\f' {

			if buffer.Len() > 0 {
				pushElem()
			}
		} else {
			buffer.WriteRune(ch)
		}
	}

	if buffer.Len() > 0 {
		pushElem()
	}

	return tokens
}

func (i *Interpreter) initPrimitives() {
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
	i.addImmediatePrimitiveToDictionary("immediate", func(inter *Interpreter) {
		inter.latest.Front().Value.(*Word).immediate = true
	})
	i.addImmediatePrimitiveToDictionary(">c", func(inter *Interpreter) {
		inter.state = false
	})
	i.addNormalPrimitiveToDictionary("<c", func(inter *Interpreter) {
		inter.state = true
	})
	i.addNormalPrimitiveToDictionary("word", func(inter *Interpreter) {
		wordContent := inter.stream.Front()
		inter.dpush(wordContent.Value.(string))
		inter.stream.Remove(wordContent)
	})
	i.addNormalPrimitiveToDictionary("create", func(inter *Interpreter) {
		wordName := inter.dpop().(string)
		inter.latest.PushFront(&Word{wordName, false, &Quotation{}})
	})
	i.addNormalPrimitiveToDictionary(",", func(inter *Interpreter) {
		currentWordBeingCompiled := inter.latest.Front().Value.(*Word).definition
		currentWordBeingCompiled.PushBack(inter.dpop())
	})
	i.addNormalPrimitiveToDictionary("'", func(inter *Interpreter) {
		wordName := inter.stream.Front().Value.(string)
		codeWord := inter.findWordInDictionary(wordName)
		inter.dpush(codeWord)
	})

	cwWord := i.findWordInDictionary("word")
	cwCreate := i.findWordInDictionary("create")
	cwCompile := i.findWordInDictionary("<c")
	cwNormal := i.findWordInDictionary(">c")

	// ':' - word definition
	colonDef := &Quotation{}
	colonDef.PushBack(cwWord)
	colonDef.PushBack(cwCreate)
	colonDef.PushBack(cwCompile)
	i.addWordToDictionary(":", false, colonDef)

	// ';' - word definition termination
	semicolonDef := &Quotation{}
	colonDef.PushBack(cwNormal)
	i.addWordToDictionary(";", true, semicolonDef)

	i.addNormalPrimitiveToDictionary("if", func(inter *Interpreter) {
		trueQuotation := inter.dpop().(*Quotation)
		falseQuotation := inter.dpop().(*Quotation)
		predicateAsStr := inter.dpop()
		predicate, ok := predicateAsStr.(bool)

		if ok {
			if predicate {
				trueQuotation.Invoke(inter)
			} else {
				falseQuotation.Invoke(inter)
			}
		} else {
			panic(fmt.Sprintf("Conditional word must be used with true/false, got %s.", predicateAsStr))
		}
	})

	i.addNormalPrimitiveToDictionary(".", func(inter *Interpreter) {
		elem := inter.dpop()
		switch t := elem.(type) {
		case Number:
			switch elem.(Number).numType {
			case IntegerType:
				fmt.Printf("%d\n", elem.(Number).AssertAsInt())
			case FloatType:
				fmt.Printf("%f\n", elem.(Number).AssertAsFloat())
			default:
				panic(fmt.Sprintf("Unsupported underlying numeric type: %d", elem.(Number).numType))
			}
		case StringLiteral:
			fmt.Printf("\"%s\"\n", elem.(StringLiteral).string)
		default:
			panic(fmt.Sprintf("Unsupported type on stack: %T (%s)\n", elem, t))
		}
	})

	i.addNormalPrimitiveToDictionary("stack", func(inter *Interpreter) {
		inter.DumpStack()
	})
}

func getFileAsString(fileName string) (string, error) {
	data, err := ioutil.ReadFile(fileName)

	if err != nil {
		return "", err
	}

	return string(data), nil
}

func (i *Interpreter) Init() {
	i.state = false
	i.stream = &list.List{}
	i.dstack = &Stack{}
	i.rstack = &Stack{}
	i.latest = list.List{}
	i.initPrimitives()

	i.Execute("init.th4")
}

func (i *Interpreter) DumpStack() {
	fmt.Printf("Stack (depth: %d)\n", i.dstack.Length())
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

	i.stream.Remove(current)

	currentWordBeingCompiled := i.latest.Front().Value.(*Word)

	dataAsString, ok := current.Value.(string)
	if ok {
		word := i.findWordInDictionary(dataAsString)

		if word != nil {
			if i.state && !word.immediate { // compile mode
				currentWordBeingCompiled.definition.PushBack(word)
			} else { // immediate mode
				word.definition.Invoke(i)
			}
		}
	}

	dataAsNumber, ok := current.Value.(Number)
	if ok {
		if i.state { // compile mode
			currentWordBeingCompiled.definition.PushBack(dataAsNumber)
		} else { // immediate mode
			i.dpush(dataAsNumber)
		}
	}

	dataAsStringLiteral, ok := current.Value.(StringLiteral)
	if ok {
		if i.state { // compile mode
			currentWordBeingCompiled.definition.PushBack(dataAsStringLiteral)
		} else { // immediate mode
			i.dpush(dataAsStringLiteral)
		}
	}

	dataAsQuotation, ok := current.Value.(*Quotation)
	if ok {
		if i.state { // compile mode
			currentWordBeingCompiled.definition.PushBack(dataAsQuotation)
		} else { // immediate mode
			i.dpush(dataAsQuotation)
		}
	}

	return true
}

func (i *Interpreter) ExecuteAsREPL(input string) {
	i.stream.PushFrontList(tokenize(input))

	for i.Step() == true {
	}
}

func (i *Interpreter) Execute(fileName string) {
	input, err := getFileAsString(fileName)

	if err != nil {
		msg := fmt.Sprintf("'%s' is not a valid throf (.th4) file. Exiting.\nError context: %s", fileName, err.Error())
		panic(msg)
	}

	// if someone loads an empty file, there's no use actually parsing it.
	if input != "" {
		i.ExecuteAsREPL(input)
	}
}

func (i *Interpreter) addPrimitiveWordToDictionary(name string, immediate bool,
	definition CodeWord) {

	word := &Word{}
	word.name = name
	word.immediate = immediate
	word.definition = &Quotation{}
	word.definition.PushBack(definition)

	i.latest.PushFront(word)
}

func (i *Interpreter) addImmediatePrimitiveToDictionary(name string, definition CodeWord) {
	i.addPrimitiveWordToDictionary(name, true, definition)
}

func (i *Interpreter) addNormalPrimitiveToDictionary(name string, definition CodeWord) {
	i.addPrimitiveWordToDictionary(name, false, definition)
}

func (i *Interpreter) addWordToDictionary(name string, immediate bool,
	definition *Quotation) {

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
