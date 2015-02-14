package main

type Stack struct {
	top  *Node
	size int
}

type Node struct {
	value interface{}
	next  *Node
}

func (s *Stack) Length() int {
	return s.size
}

func (s *Stack) IsEmpty() bool {
	return s.size == 0
}

func (s *Stack) Push(val interface{}) {
	s.top = &Node{val, s.top}
	s.size++
}

func (s *Stack) Peek() interface{} {
	return s.top.value
}

func (s *Stack) Pop() (val interface{}) {
	if s.size > 0 {
		val, s.top = s.top.value, s.top.next
		s.size--
		return
	}
	return nil
}

func (s *Stack) InsertAfter(depth int, val interface{}) {

	current := s.top
	for ii := 0; ii < depth; ii++ {
		current = current.next
	}

	elem := &Node{val, current.next}
	current.next = elem
	s.size++
}

func (s *Stack) GetAt(depth int) interface{} {
	current := s.top
	for ii := 0; ii < depth; ii++ {
		current = current.next
	}

	return current.value
}

func (s *Stack) RemoveAt(depth int) interface{} {
	previousNode := s.top
	for ii := 0; ii < depth-1; ii++ {
		previousNode = previousNode.next
	}

	elem := previousNode.next.value
	previousNode.next = previousNode.next.next
	s.size--

	return elem
}
