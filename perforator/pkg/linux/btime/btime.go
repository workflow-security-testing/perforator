package btime

import (
	"errors"
	"time"

	"golang.org/x/sys/unix"
)

var bootTime time.Time

func init() {
	bootTimeMicros, err := getBootTimeMicros()
	if err == nil {
		bootTime = time.Unix(0, bootTimeMicros*int64(time.Microsecond))
	}
}

// Returns boot time with microsecond-ish precision.
func GetBootTime() (time.Time, error) {
	if bootTime.IsZero() {
		return time.Time{}, errors.New("boot time was not initialized successfully")
	}

	return bootTime, nil
}

// getBootTimeMicros returns boot time in microseconds since Unix epoch.
// This function uses clock_gettime(CLOCK_REALTIME) and clock_gettime(CLOCK_BOOTTIME)
// to calculate boot time with microsecond precision.
func getBootTimeMicros() (int64, error) {
	var realtime, boottime unix.Timespec

	err := unix.ClockGettime(unix.CLOCK_REALTIME, &realtime)
	if err != nil {
		return 0, err
	}

	err = unix.ClockGettime(unix.CLOCK_BOOTTIME, &boottime)
	if err != nil {
		return 0, err
	}

	const nanosecondsPerSecond = 1_000_000_000
	realtimeNanos := realtime.Sec*nanosecondsPerSecond + realtime.Nsec
	boottimeNanos := boottime.Sec*nanosecondsPerSecond + boottime.Nsec

	bootTimeNanoseconds := realtimeNanos - boottimeNanos

	const nanosecondsPerMicrosecond = 1_000

	return bootTimeNanoseconds / nanosecondsPerMicrosecond, nil
}
