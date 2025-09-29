package custom_profiling_operation

import (
	"errors"
	"time"

	cpo_proto "github.com/yandex/perforator/perforator/proto/custom_profiling_operation"
	"github.com/yandex/perforator/perforator/proto/lib/time_interval"
)

func validateTimeInterval(timeInterval *time_interval.TimeInterval) error {
	if timeInterval == nil || timeInterval.From == nil || timeInterval.To == nil {
		return errors.New("time interval is not set")
	}

	if timeInterval.From.AsTime().IsZero() {
		return errors.New("time interval is invalid: start is zero")
	}

	if timeInterval.To.AsTime().IsZero() {
		return errors.New("time interval is invalid: end is zero")
	}

	if timeInterval.From.AsTime().After(timeInterval.To.AsTime()) {
		return errors.New("time interval is invalid: start is after end")
	}

	if timeInterval.To.AsTime().Before(time.Now()) {
		return errors.New("time interval is in the past")
	}

	return nil
}

func validateOperationSpec(spec *cpo_proto.OperationSpec) error {
	if spec.Target == nil || spec.Target.Target == nil {
		return errors.New("target is not set")
	}

	if spec.Event == nil || spec.Event.Settings == nil {
		return errors.New("event is not set")
	}

	err := validateTimeInterval(spec.TimeInterval)
	if err != nil {
		return err
	}

	return nil
}
