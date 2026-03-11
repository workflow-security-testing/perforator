package signal_profile_processor

import (
	"context"
	"errors"
	"fmt"
	"time"

	"github.com/yandex/perforator/library/go/ptr"
	"github.com/yandex/perforator/perforator/pkg/profile_event"
	"github.com/yandex/perforator/perforator/pkg/profilequerylang"
	proto "github.com/yandex/perforator/perforator/proto/perforator"
	"github.com/yandex/perforator/perforator/symbolizer/pkg/client"
)

type Processor interface {
	Process(ctx context.Context, in *profile_event.SignalProfileEvent) (*profile_event.CoreMessage, error)
}

type ProxyProcessor struct {
	client *client.Client
}

func NewProxyProcessor(c *client.Client) *ProxyProcessor {
	return &ProxyProcessor{client: c}
}

func (p *ProxyProcessor) Process(ctx context.Context, in *profile_event.SignalProfileEvent) (*profile_event.CoreMessage, error) {
	sel, err := selectorFromEvent(in)
	if err != nil {
		return nil, err
	}

	format := textRenderFormat()

	tracebackText, _, err := p.client.MergeProfiles(ctx, &client.MergeProfilesRequest{
		ProfileFilters: client.ProfileFilters{
			Selector: sel.Selector,
			FromTS:   sel.From,
			ToTS:     sel.To,
		},
		Format: format,
	},
		false, // asURL (we want to get profile itself)
		fmt.Sprintf("profile-event-processor, profile_id=%q", in.ProfileID),
	)
	if err != nil {
		return nil, err
	}

	if len(in.SignalTypes) == 0 {
		return nil, errors.New("no signal types provided")
	}

	core := &profile_event.CoreEvent{
		Service:   in.Service,
		Cluster:   in.Cluster,
		PodID:     in.PodID,
		NodeID:    in.NodeID,
		Signal:    in.SignalTypes[0], // TODO: there might be several signal types in the same profile.
		Message:   "perforator text profile",
		Timestamp: in.Timestamp.Unix(),
		Traceback: string(tracebackText),
	}

	msg := &profile_event.CoreMessage{
		PartitionKey: in.Service,
		Event: &profile_event.CoreEventMessage{
			Core: core,
		},
	}
	return msg, nil
}

func textRenderFormat() *proto.RenderFormat {
	return &proto.RenderFormat{
		Symbolize: &proto.SymbolizeOptions{
			Symbolize: ptr.Bool(true),
		},
		Postprocessing: &proto.PostprocessOptions{
			MergePythonAndNativeStacks: ptr.Bool(true),
		},
		Format: &proto.RenderFormat_TextProfile{
			TextProfile: &proto.TextProfileOptions{
				ShowFileNames:   ptr.Bool(true),
				ShowLineNumbers: ptr.Bool(true),
				MaxSamples:      ptr.Uint32(0),
			},
		},
	}
}

func selectorFromEvent(ev *profile_event.SignalProfileEvent) (timeLimitedSelector, error) {
	fromTS := ev.Timestamp.Add(-24 * time.Hour)
	toTS := ev.Timestamp.Add(24 * time.Hour)
	b := profilequerylang.NewBuilder().
		ProfileIDs(ev.ProfileID).
		From(fromTS).
		To(toTS)
	sel := b.Build()
	b.AddMatcher(sel, profilequerylang.EventTypeLabel, []string{ev.MainEvent})

	str, err := profilequerylang.SelectorToString(sel)
	if err != nil {
		return timeLimitedSelector{}, fmt.Errorf("selector build: %w", err)
	}
	return timeLimitedSelector{
		Selector: str,
		From:     fromTS,
		To:       toTS,
	}, nil
}

type timeLimitedSelector struct {
	Selector string
	From     time.Time
	To       time.Time
}
