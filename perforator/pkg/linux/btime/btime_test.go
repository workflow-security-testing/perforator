package btime

import (
	"testing"

	"github.com/stretchr/testify/require"
)

func TestBootTimeInitialized(t *testing.T) {
	btime, err := GetBootTime()
	require.NoError(t, err)
	require.False(t, btime.IsZero())
}
