package querylang

import (
	"strings"
)

// Expression may represent only a subset of possible expressions of Solomon language.
type Expression struct {
	// one of:
	FunctionCall *FunctionCall
	Selector     *Selector
	Value        Value
}

type FunctionCall struct {
	Identifier string
	Arguments  []*Expression
}

func (f *FunctionCall) Repr() string {
	argsReprs := make([]string, len(f.Arguments))
	for i, arg := range f.Arguments {
		argsReprs[i] = arg.Repr()
	}
	return f.Identifier + "(" + strings.Join(argsReprs, ", ") + ")"
}

func (e *Expression) Repr() string {
	switch {
	case e.FunctionCall != nil:
		return e.FunctionCall.Repr()
	case e.Selector != nil:
		sr := e.Selector.Repr()
		if sr != "" {
			return sr
		}
		return "empty_selector"
	case e.Value != nil:
		return e.Value.Repr()
	}
	return "invalid_expression"
}
