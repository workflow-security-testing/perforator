package profiler_test

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"

	"github.com/yandex/perforator/perforator/internal/xmetrics"
)

type tags map[string]string

func metric(m xmetrics.Registry, name string, tags tags) float64 {
	buf := new(bytes.Buffer)
	err := m.StreamMetrics(context.Background(), buf)
	if err != nil {
		panic(err)
	}

	var res struct {
		Metrics []struct {
			Type   string            `json:"type"`
			Labels map[string]string `json:"labels"`
			Value  float64           `json:"value"`
		} `json:"metrics"`
	}

	err = json.Unmarshal(buf.Bytes(), &res)
	if err != nil {
		panic(err)
	}

	for _, metric := range res.Metrics {
		if metric.Labels["sensor"] != name {
			continue
		}

		ok := true
		for k, v := range tags {
			if metric.Labels[k] != v {
				ok = false
				break
			}
		}
		if !ok {
			continue
		}

		return metric.Value
	}

	panic(fmt.Sprintf("metric %s with tags %v was not found", name, tags))
}
