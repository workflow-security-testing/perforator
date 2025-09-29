package profile_event

import "time"

const ServiceKey = "service"

type SignalProfileMessage struct {
	PartitionKey string
	Event        *SignalProfileEvent
}

type SignalProfileEvent struct {
	ProfileID   string    `json:"profile_id"`
	Service     string    `json:"service"`
	Cluster     string    `json:"cluster"`
	NodeID      string    `json:"node_id"`
	PodID       string    `json:"pod_id"`
	Timestamp   time.Time `json:"timestamp"`
	BuildIDs    []string  `json:"build_ids"`
	MainEvent   string    `json:"main_event"`
	SignalTypes []string  `json:"signal_types"`
}

type CoreMessage struct {
	PartitionKey string
	Event        *CoreEvent
}

const CoreEventServiceKey = "service"

type CoreEvent struct {
	Service    string            `json:"service"`
	Type       string            `json:"type"`
	Cluster    string            `json:"cluster"`
	PodID      string            `json:"pod_id"`
	NodeID     string            `json:"node_id"`
	Signal     string            `json:"signal"`
	Message    string            `json:"message"`
	Timestamp  int64             `json:"timestamp"`
	Attributes map[string]string `json:"attributes"`
	Traceback  string            `json:"traceback"`
}
