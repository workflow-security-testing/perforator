package render

import (
	"encoding/json"
	"io"

	"github.com/yandex/perforator/perforator/pkg/profile/flamegraph/render/format"
)

// JSONRenderer produces flamegraph JSON output.
// This interface allows plugging in different JSON generation backends
// (e.g., Go implementation using blocks, or C++ implementation via CGO).
type JSONRenderer interface {
	// RenderJSON writes the flamegraph data as JSON to the writer.
	RenderJSON(w io.Writer) error
}

////////////////////////////////////////////////////////////////////////////////

// BlocksJSONRenderer produces flamegraph JSON from pre-built blocks.
// This is the Go implementation of JSONRenderer.
type BlocksJSONRenderer struct {
	blocks    []*block
	eventType string
	frameType string
}

// NewBlocksJSONRenderer creates a new BlocksJSONRenderer.
func NewBlocksJSONRenderer(blocks []*block, eventType, frameType string) *BlocksJSONRenderer {
	return &BlocksJSONRenderer{
		blocks:    blocks,
		eventType: eventType,
		frameType: frameType,
	}
}

// RenderJSON implements JSONRenderer.
func (r *BlocksJSONRenderer) RenderJSON(w io.Writer) error {
	enc := json.NewEncoder(w)
	return r.encodeToJSON(enc)
}

// RenderPrettyJSON renders JSON with indentation.
func (r *BlocksJSONRenderer) RenderPrettyJSON(w io.Writer) error {
	enc := json.NewEncoder(w)
	enc.SetIndent("", "  ")
	return r.encodeToJSON(enc)
}

func (r *BlocksJSONRenderer) encodeToJSON(enc *json.Encoder) error {
	strtab := NewStringTable()

	maxLevel := 0
	for _, block := range r.blocks {
		if block.level > maxLevel {
			maxLevel = block.level
		}
	}

	nodeLevels := make([][]format.RenderingNode, maxLevel+1)
	blocksByLevels := populateWithIndexes(r.blocks[0], maxLevel+1, len(r.blocks))

	for _, blocksOnLevel := range blocksByLevels {
		for _, currentBlock := range blocksOnLevel {
			parentIndex := -1
			if currentBlock.parent != nil {
				parentIndex = currentBlock.parent.levelPos
			}
			node := format.RenderingNode{
				ParentIndex:     parentIndex,
				TextID:          strtab.Add(currentBlock.name),
				SampleCount:     currentBlock.nextCount.count,
				EventCount:      currentBlock.nextCount.events,
				BaseEventCount:  currentBlock.prevCount.events,
				BaseSampleCount: currentBlock.prevCount.count,
				FrameOrigin:     strtab.Add(string(currentBlock.frameOrigin)),
				Kind:            strtab.Add(currentBlock.kind),
				File:            strtab.Add(currentBlock.file),
				Inlined:         currentBlock.inlined,
			}
			nodeLevels[currentBlock.level] = append(nodeLevels[currentBlock.level], node)
		}
	}

	profileMeta := format.ProfileMeta{
		EventType: strtab.Add(r.eventType),
		FrameType: strtab.Add(r.frameType),
		Version:   2,
	}

	profileData := format.ProfileData{
		Nodes:   nodeLevels,
		Strings: strtab.Table(),
		Meta:    profileMeta,
	}

	return enc.Encode(profileData)
}
