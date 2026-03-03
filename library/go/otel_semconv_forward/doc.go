// Package semconv is a stable import path forwarding to OTel semconv.
package semconv

//go:generate ya tool go run ./internal/cmd/gensemconv -out semconv_gen.go
