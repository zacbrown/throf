package main

import (
	"math"
	"regexp"
)

type Numeric interface {
	Add(rhs Numeric) Numeric
	Sub(rhs Numeric) Numeric
	Mul(rhs Numeric) Numeric
	Div(rhs Numeric) Numeric
}

type NumericType byte

const (
	IntegerType  NumericType = iota
	FloatType                = iota
	RationalType             = iota
)

type Number struct {
	val     interface{}
	numType NumericType
}

func NewInt(val int) Number {
	return Number{val, IntegerType}
}

func NewFloat(val float64) Number {
	return Number{val, FloatType}
}

func (n Number) AssertAsInt() int {
	return n.val.(int)
}

func (n Number) AssertAsFloat() float64 {
	return n.val.(float64)
}

func (n Number) CoerceToFloat() float64 {
	var num float64
	if n.numType == IntegerType {
		num = float64(n.val.(int))
	} else {
		num = n.val.(float64)
	}

	return num
}

func (lhs Number) Add(rhs Number) Number {
	coerceToFloat := lhs.numType == FloatType || rhs.numType == FloatType

	if coerceToFloat {
		newLhs := lhs.CoerceToFloat()
		newRhs := rhs.CoerceToFloat()

		ret := newLhs + newRhs
		return Number{ret, FloatType}
	}

	newLhs := lhs.val.(int)
	newRhs := rhs.val.(int)
	return Number{(newLhs + newRhs), IntegerType}
}

func (lhs Number) Sub(rhs Number) Number {
	coerceToFloat := lhs.numType == FloatType || rhs.numType == FloatType

	if coerceToFloat {
		newLhs := lhs.CoerceToFloat()
		newRhs := rhs.CoerceToFloat()

		ret := newLhs - newRhs
		return Number{ret, FloatType}
	}

	newLhs := lhs.val.(int)
	newRhs := rhs.val.(int)
	return Number{(newLhs - newRhs), IntegerType}
}

func (lhs Number) Mul(rhs Number) Number {
	coerceToFloat := lhs.numType == FloatType || rhs.numType == FloatType

	if coerceToFloat {
		newLhs := lhs.CoerceToFloat()
		newRhs := rhs.CoerceToFloat()

		ret := newLhs * newRhs
		return Number{ret, FloatType}
	}

	newLhs := lhs.val.(int)
	newRhs := rhs.val.(int)
	return Number{(newLhs * newRhs), IntegerType}
}

func (lhs Number) Div(rhs Number) Number {
	// Division is a special snowflake. Dividing
	// an integer by an integer might result in a float
	// or it might result in an int. We go straight into
	// coercing to float then determine if we can reduce to
	// int.
	newLhs := lhs.CoerceToFloat()
	newRhs := rhs.CoerceToFloat()

	retVal := newLhs / newRhs
	integer, fraction := math.Modf(retVal)

	if fraction == 0.0 {
		return Number{int(integer), IntegerType}
	}

	return Number{retVal, FloatType}
}

func (lhs Number) Mod(rhs Number) Number {
	newLhs := lhs.CoerceToFloat()
	newRhs := rhs.CoerceToFloat()

	retVal := math.Mod(newLhs, newRhs)

	return Number{retVal, FloatType}
}

func (lhs Number) Equals(rhs Number) bool {
	newLhs := lhs.CoerceToFloat()
	newRhs := rhs.CoerceToFloat()

	return newLhs == newRhs
}
}
