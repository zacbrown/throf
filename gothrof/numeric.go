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

func (n Number) AsInt() int {
	return n.val.(int)
}

func (n Number) AsFloat() float64 {
	return n.val.(float64)
}

func (lhs Number) Add(rhs Number) Number {
	coerceToFloat := lhs.numType == FloatType || rhs.numType == FloatType

	if coerceToFloat {
		var newLhs float64
		if lhs.numType == IntegerType {
			newLhs = float64(lhs.AsInt())
		} else {
			newLhs = lhs.AsFloat()
		}

		var newRhs float64
		if rhs.numType == IntegerType {
			newRhs = float64(rhs.AsInt())
		} else {
			newRhs = rhs.AsFloat()
		}

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
		var newLhs float64
		if lhs.numType == IntegerType {
			newLhs = float64(lhs.AsInt())
		} else {
			newLhs = lhs.AsFloat()
		}

		var newRhs float64
		if rhs.numType == IntegerType {
			newRhs = float64(rhs.AsInt())
		} else {
			newRhs = rhs.AsFloat()
		}

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
		var newLhs float64
		if lhs.numType == IntegerType {
			newLhs = float64(lhs.AsInt())
		} else {
			newLhs = lhs.AsFloat()
		}

		var newRhs float64
		if rhs.numType == IntegerType {
			newRhs = float64(rhs.AsInt())
		} else {
			newRhs = rhs.AsFloat()
		}

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
	var newLhs float64
	if lhs.numType == IntegerType {
		newLhs = float64(lhs.AsInt())
	} else {
		newLhs = lhs.AsFloat()
	}

	var newRhs float64
	if rhs.numType == IntegerType {
		newRhs = float64(rhs.AsInt())
	} else {
		newRhs = rhs.AsFloat()
	}

	retVal := newLhs / newRhs
	integer, fraction := math.Modf(retVal)

	if fraction == 0.0 {
		return Number{int(integer), IntegerType}
	}

	return Number{retVal, FloatType}

}
}
