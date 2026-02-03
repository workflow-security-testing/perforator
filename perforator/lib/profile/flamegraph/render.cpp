#include "render.h"

#include <perforator/lib/profile/merge.h>
#include <perforator/lib/profile/profile.h>
#include <perforator/lib/profile/trie/trie.h>

#include <library/cpp/containers/absl_flat_hash/flat_hash_map.h>

#include <contrib/libs/rapidjson/include/rapidjson/writer.h>
#include <contrib/libs/rapidjson/include/rapidjson/stringbuffer.h>

#include <util/generic/overloaded.h>
#include <util/memory/pool.h>

#include <algorithm>
#include <limits>

namespace NPerforator::NProfile {

////////////////////////////////////////////////////////////////////////////////

namespace {

// Frame identity for deduplication
struct TFrameKey {
    TStringId NameId = TStringId::Invalid();
    TStringId FileId = TStringId::Invalid();
    TBinaryId BinaryId = TBinaryId::Zero();
    ui32 Line = 0;
    ui32 Column = 0;

    bool operator==(const TFrameKey& other) const = default;

    template <typename H>
    friend H AbslHashValue(H h, const TFrameKey& key) {
        return H::combine(std::move(h), key.NameId, key.FileId, key.BinaryId, key.Line, key.Column);
    }
};

// Flamegraph node identity - root, label, or frame
using TFlameNodeId = std::variant<std::monostate, TLabelId, TFrameKey>;

struct TFlameValue {
    i64 SampleCount = 0;
    i64 EventCount = 0;

    TFlameValue& operator+=(const TFlameValue& other) {
        SampleCount += other.SampleCount;
        EventCount += other.EventCount;
        return *this;
    }
};

using TFlameTrie = TProfileTrie<TFlameNodeId, TFlameValue>;

////////////////////////////////////////////////////////////////////////////////

class TLabelKeyIds {
public:
    static TLabelKeyIds Build(TProfile& profile) {
        TLabelKeyIds ids;

        THashMap<TStringBuf, NProto::NProfile::WellKnownLabel> keyToLabel;
        for (auto label : TProfile::GetWellKnownLabels()) {
            for (const TString& key : TProfile::GetAllWellKnownLabelKeys(label)) {
                keyToLabel.emplace(key, label);
            }
        }

        for (TStringRef str : profile.Strings()) {
            if (auto it = keyToLabel.find(str.View()); it != keyToLabel.end()) {
                ids.KeyToLabel_.emplace(str.GetIndex(), it->second);
            }
        }

        return ids;
    }

    std::optional<NProto::NProfile::WellKnownLabel> GetLabel(TStringId keyId) const {
        auto it = KeyToLabel_.find(keyId);
        return it != KeyToLabel_.end() ? std::optional{it->second} : std::nullopt;
    }

private:
    absl::flat_hash_map<TStringId, NProto::NProfile::WellKnownLabel> KeyToLabel_;
};

////////////////////////////////////////////////////////////////////////////////

bool IsInvalidFunctionName(TStringBuf name) {
    return name.empty() || name == "??" || name == "<invalid>";
}

bool IsInvalidFilename(TStringBuf name) {
    return name.empty() || name == "??" || name == "<invalid>" || name == "<unknown>";
}

////////////////////////////////////////////////////////////////////////////////

TFlameTrie BuildFlameTrie(
    TProfile& profile,
    const TLabelKeyIds& keyIds,
    const NProto::NProfile::RenderOptions& options
) {
    TFlameTrie trie{std::monostate{}};

    TVector<TFlameValue> keyValues(profile.SampleKeys().Size());
    for (auto sample : profile.Samples()) {
        ui32 keyIndex = sample.GetKey().GetIndex().GetInternalIndex();
        keyValues[keyIndex].SampleCount += 1;
        keyValues[keyIndex].EventCount += sample.GetValue(0);
    }

    // Process each sample key (unique stack)
    for (auto sampleKey : profile.SampleKeys()) {
        ui32 keyIndex = sampleKey.GetIndex().GetInternalIndex();
        TFlameValue value = keyValues[keyIndex];

        if (value.SampleCount == 0) {
            continue;
        }

        ui32 nodeIdx = 0;  // Root
        trie.AddValue(nodeIdx, value);
        ui32 depth = 0;    // Track depth for truncation (includes labels, like Go)

        // Helper to descend and accumulate value
        auto descend = [&](TFlameNodeId id) {
            nodeIdx = trie.GetOrCreateChild(nodeIdx, id);
            trie.AddValue(nodeIdx, value);
            ++depth;
        };

        // Process labels
        bool hasFirstContainer = false;
        std::array<TLabelId, 5> labelNodes{{
            TLabelId::Invalid(),
            TLabelId::Invalid(),
            TLabelId::Invalid(),
            TLabelId::Invalid(),
            TLabelId::Invalid(),
        }};

        for (auto label : sampleKey.GetAllLabels()) {
            TStringId keyId = label.GetKey().GetIndex();
            auto labelType = keyIds.GetLabel(keyId);

            if (!labelType) {
                continue;
            }

            switch (*labelType) {
                case NProto::NProfile::Workload:
                    if (label.IsString()) {
                        if (!hasFirstContainer && label.GetString().View().StartsWith("iss_hook_")) {
                            hasFirstContainer = true;
                            break;
                        }
                        hasFirstContainer = true;
                        descend(TFlameNodeId{label.GetIndex()});
                    }
                    break;
                case NProto::NProfile::ProcessId:
                    if (label.IsNumber()) {
                        labelNodes[0] = label.GetIndex();
                    }
                    break;
                case NProto::NProfile::ProcessCommand:
                    if (label.IsString() && label.GetString().GetIndex() != TStringId::Zero()) {
                        labelNodes[1] = label.GetIndex();
                    }
                    break;
                case NProto::NProfile::ThreadId:
                    if (label.IsNumber()) {
                        labelNodes[2] = label.GetIndex();
                    }
                    break;
                case NProto::NProfile::ThreadCommand:
                    if (label.IsString() && label.GetString().GetIndex() != TStringId::Zero()) {
                        labelNodes[3] = label.GetIndex();
                    }
                    break;
                case NProto::NProfile::SignalName:
                    if (label.IsString() && label.GetString().GetIndex() != TStringId::Zero()) {
                        labelNodes[4] = label.GetIndex();
                    }
                    break;
                default:
                    break;
            }
        }

        for (TLabelId labelId : labelNodes) {
            if (labelId.IsValid()) {
                descend(TFlameNodeId{labelId});
            }
        }

        // Process stacks (reverse order - root to leaf)
        bool truncated = false;
        for (i32 stackIdx = sampleKey.GetStackCount() - 1; stackIdx >= 0 && !truncated; --stackIdx) {
            TStack stack = sampleKey.GetStack(stackIdx);
            for (i32 frameIdx = stack.GetFrameCount() - 1; frameIdx >= 0 && !truncated; --frameIdx) {
                TStackFrame frame = stack.GetFrame(frameIdx);
                TBinaryId binaryId = frame.GetBinary().GetIndex();
                TStringId binaryPathId = binaryId != TBinaryId::Zero()
                    ? frame.GetBinary().GetPath().GetIndex()
                    : TStringId::Invalid();

                TInlineChain chain = frame.GetInlineChain();
                if (chain.GetLineCount() == 0) {
                    if (options.max_depth() > 0 && depth >= options.max_depth()) {
                        truncated = true;
                        break;
                    }
                    descend(TFlameNodeId{TFrameKey{
                        .NameId = TStringId::Invalid(),
                        .FileId = binaryPathId,
                        .BinaryId = binaryId,
                        .Line = 0,
                        .Column = 0,
                    }});
                } else {
                    for (i32 lineIdx = 0; lineIdx < chain.GetLineCount(); ++lineIdx) {
                        if (options.max_depth() > 0 && depth >= options.max_depth()) {
                            truncated = true;
                            break;
                        }
                        TSourceLine line = chain.GetLine(lineIdx);
                        TFunction func = line.GetFunction();
                        TStringId fileId = func.GetFileName().GetIndex();
                        if (IsInvalidFilename(func.GetFileName().View())) {
                            fileId = binaryPathId;
                        }
                        descend(TFlameNodeId{TFrameKey{
                            .NameId = func.GetName().GetIndex(),
                            .FileId = fileId,
                            .BinaryId = binaryId,
                            .Line = line.GetLine(),
                            .Column = line.GetColumn(),
                        }});
                    }
                }
            }
        }
    }

    trie.Finalize();
    return trie;
}

////////////////////////////////////////////////////////////////////////////////

// String table for JSON output - pool provides stable memory-local storage
class TStringInterner {
public:
    TStringInterner()
        : Pool_(4096)
    {}

    ui32 Intern(TStringBuf s) {
        if (auto it = StringToId_.find(s); it != StringToId_.end()) {
            return it->second;
        }
        ui32 id = Strings_.size();
        TStringBuf interned = Pool_.AppendString(s);
        Strings_.push_back(interned);
        StringToId_.emplace(interned, id);
        return id;
    }

    TStringBuf Get(ui32 id) const {
        return Strings_[id];
    }

    template<typename F>
    void ForEach(F&& f) const {
        for (TStringBuf s : Strings_) {
            f(s);
        }
    }

private:
    TMemoryPool Pool_;
    TVector<TStringBuf> Strings_;
    absl::flat_hash_map<TStringBuf, ui32> StringToId_;
};

// Common string IDs for rendering
struct TCommonStrings {
    ui32 Empty;
    ui32 All;
    ui32 Container;
    ui32 Process;
    ui32 Thread;
    ui32 Signal;
    ui32 Native;
    ui32 Kernel;
    ui32 Python;
    ui32 Php;
    ui32 UnsymbolizedFunction;
    ui32 UnknownMapping;
    ui32 Samples;
    ui32 Function;
    ui32 UnsymbolizedAddress;  // "??"

    static TCommonStrings Build(TStringInterner& table) {
        return TCommonStrings{
            .Empty = table.Intern(""),
            .All = table.Intern("all"),
            .Container = table.Intern("container"),
            .Process = table.Intern("process"),
            .Thread = table.Intern("thread"),
            .Signal = table.Intern("signal"),
            .Native = table.Intern("native"),
            .Kernel = table.Intern("kernel"),
            .Python = table.Intern("python"),
            .Php = table.Intern("php"),
            .UnsymbolizedFunction = table.Intern("<unsymbolized function>"),
            .UnknownMapping = table.Intern("<unknown mapping>"),
            .Samples = table.Intern("samples"),
            .Function = table.Intern("Function"),
            .UnsymbolizedAddress = table.Intern("??"),
        };
    }

    ui32 GetOriginId(TStringBuf binaryPath) const {
        if (binaryPath == "[kernel]") {
            return Kernel;
        } else if (binaryPath == "[python]") {
            return Python;
        } else if (binaryPath == "[php]") {
            return Php;
        }
        return Native;
    }
};

// Rendered node data
struct TRenderedNode {
    ui32 NameId;
    ui32 FileId;
    ui32 OriginId;
    ui32 KindId;
};

// Node renderer - converts raw IDs to interned strings
// Derives NodeKind from label keys at render time
class TNodeRenderer {
public:
    TNodeRenderer(
        TProfile& profile,
        TStringInterner& stringTable,
        const TCommonStrings& common,
        const TLabelKeyIds& keyIds)
        : Profile_(profile)
        , StringTable_(stringTable)
        , Common_(common)
        , KeyIds_(keyIds)
    {}

    TRenderedNode Render(const TFlameNodeId& identity) {
        return std::visit(TOverloaded{
            [this](std::monostate) {
                return TRenderedNode{
                    .NameId = Common_.All,
                    .FileId = Common_.Empty,
                    .OriginId = Common_.Empty,
                    .KindId = Common_.Empty,
                };
            },
            [this](TLabelId labelId) { return RenderLabel(labelId); },
            [this](const TFrameKey& frame) { return RenderFrame(frame); },
        }, identity);
    }

private:
    TRenderedNode RenderLabel(TLabelId labelId) {
        TRenderedNode result{
            .NameId = Common_.Empty,
            .FileId = Common_.Empty,
            .OriginId = Common_.Empty,
            .KindId = Common_.Empty,
        };

        TLabel label = Profile_.Labels().Get(labelId);
        TStringId keyId = label.GetKey().GetIndex();
        auto labelType = KeyIds_.GetLabel(keyId);

        if (!labelType) {
            return result;
        }

        switch (*labelType) {
            case NProto::NProfile::Workload:
                result.NameId = StringTable_.Intern(label.GetString().View());
                result.KindId = Common_.Container;
                break;
            case NProto::NProfile::ProcessId:
                result.NameId = StringTable_.Intern(ToString(label.GetNumber()));
                result.KindId = Common_.Process;
                break;
            case NProto::NProfile::ProcessCommand:
                result.NameId = StringTable_.Intern(label.GetString().View());
                result.KindId = Common_.Process;
                break;
            case NProto::NProfile::ThreadId:
                result.NameId = StringTable_.Intern(ToString(label.GetNumber()));
                result.KindId = Common_.Thread;
                break;
            case NProto::NProfile::ThreadCommand:
                result.NameId = StringTable_.Intern(label.GetString().View());
                result.KindId = Common_.Thread;
                break;
            case NProto::NProfile::SignalName:
                result.NameId = StringTable_.Intern(label.GetString().View());
                result.KindId = Common_.Signal;
                break;
            default:
                break;
        }

        return result;
    }

    TRenderedNode RenderFrame(const TFrameKey& frame) {
        TRenderedNode result{
            .NameId = Common_.Empty,
            .FileId = Common_.Empty,
            .OriginId = Common_.Empty,
            .KindId = Common_.Empty,
        };

        TStringBuf binaryPath = frame.BinaryId != TBinaryId::Zero()
            ? Profile_.Binaries().Get(frame.BinaryId).GetPath().View()
            : TStringBuf{};

        if (frame.NameId.IsValid()) {
            TStringBuf name = Profile_.Strings().Get(frame.NameId).View();
            if (!IsInvalidFunctionName(name)) {
                result.NameId = StringTable_.Intern(name);
            }
        }

        if (frame.FileId.IsValid()) {
            TStringBuf file = Profile_.Strings().Get(frame.FileId).View();
            if (!IsInvalidFilename(file)) {
                result.FileId = StringTable_.Intern(file);
            }
        }

        if (result.NameId == Common_.Empty) {
            if (frame.BinaryId == TBinaryId::Zero()) {
                result.NameId = Common_.UnknownMapping;
            } else if (!frame.NameId.IsValid()) {
                result.NameId = Common_.UnsymbolizedAddress;
            } else {
                result.NameId = Common_.UnsymbolizedFunction;
            }
        }

        result.OriginId = Common_.GetOriginId(binaryPath);
        return result;
    }

    TProfile& Profile_;
    TStringInterner& StringTable_;
    const TCommonStrings& Common_;
    const TLabelKeyIds& KeyIds_;
};

template <typename Writer, size_t N>
void WriteKey(Writer& writer, const char (&key)[N]) {
    writer.Key(key, N - 1);  // N includes null terminator
}

template <typename Writer>
void WriteNodeJson(
    Writer& writer,
    const TRenderedNode& rendered,
    i32 parentLevelIdx,
    i64 sampleCount,
    i64 eventCount
) {
    writer.StartObject();
    WriteKey(writer, "parentIndex");
    writer.Int(parentLevelIdx);
    WriteKey(writer, "textId");
    writer.Uint(rendered.NameId);
    WriteKey(writer, "sampleCount");
    writer.Int64(sampleCount);
    WriteKey(writer, "eventCount");
    writer.Int64(eventCount);
    WriteKey(writer, "frameOrigin");
    writer.Uint(rendered.OriginId);
    WriteKey(writer, "kind");
    writer.Uint(rendered.KindId);
    WriteKey(writer, "file");
    writer.Uint(rendered.FileId);
    writer.EndObject();
}

void RenderTrieToJson(
    const TFlameTrie& trie,
    TProfile& profile,
    const TLabelKeyIds& keyIds,
    IOutputStream& out,
    const NProto::NProfile::RenderOptions& options
) {
    TStringInterner stringTable;
    TCommonStrings common = TCommonStrings::Build(stringTable);

    TNodeRenderer renderer(profile, stringTable, common, keyIds);

    // Cache rendered nodes
    TVector<TRenderedNode> renderedNodes(trie.NodeCount());
    for (ui32 i = 0; i < trie.NodeCount(); ++i) {
        renderedNodes[i] = renderer.Render(trie.GetIdentity(i));
    }

    // Calculate minWeight threshold based on root event count
    const i64 rootEventCount = trie.GetValue(0).EventCount;
    const i64 minEventThreshold = options.min_weight() > 0.0
        ? static_cast<i64>(rootEventCount * options.min_weight())
        : 0;

    // Pre-intern "(truncated stack)" for filtered children aggregation
    const ui32 truncatedNameId = minEventThreshold > 0
        ? stringTable.Intern("(truncated stack)")
        : 0;

    rapidjson::StringBuffer buffer;
    buffer.Reserve(trie.NodeCount() * 100);  // ~100 bytes per node estimate
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    writer.StartObject();
    WriteKey(writer, "rows");
    writer.StartArray();

    // Level-order BFS traversal
    // Each entry: (nodeIdx, parentLevelIdx) where nodeIdx=UINT32_MAX means truncated stack
    constexpr ui32 TruncatedNodeMarker = std::numeric_limits<ui32>::max();

    struct TLevelEntry {
        ui32 NodeIdx;
        i32 ParentLevelIdx;
        i64 SampleCount;  // Only used for truncated nodes
        i64 EventCount;   // Only used for truncated nodes
    };

    TVector<TLevelEntry> currentLevel;
    TVector<TLevelEntry> nextLevel;
    TVector<ui32> children;

    currentLevel.push_back({0, -1, 0, 0});  // Root has parent -1

    while (!currentLevel.empty()) {
        writer.StartArray();

        nextLevel.clear();

        for (size_t levelIdx = 0; levelIdx < currentLevel.size(); ++levelIdx) {
            const auto& entry = currentLevel[levelIdx];

            // Check if this is a truncated stack node
            if (entry.NodeIdx == TruncatedNodeMarker) {
                TRenderedNode truncatedNode{
                    .NameId = truncatedNameId,
                    .FileId = 0,  // empty
                    .OriginId = common.Native,
                    .KindId = common.Function,
                };
                WriteNodeJson(writer, truncatedNode, entry.ParentLevelIdx,
                             entry.SampleCount, entry.EventCount);
                continue;  // Truncated nodes have no children
            }

            const auto& nodeValue = trie.GetValue(entry.NodeIdx);
            WriteNodeJson(writer, renderedNodes[entry.NodeIdx], entry.ParentLevelIdx,
                         nodeValue.SampleCount, nodeValue.EventCount);

            children.clear();
            i64 truncatedSampleCount = 0;
            i64 truncatedEventCount = 0;

            for (ui32 child = trie.GetFirstChild(entry.NodeIdx); child != 0; child = trie.GetNextSibling(child)) {
                const auto& childValue = trie.GetValue(child);
                if (minEventThreshold > 0 && childValue.EventCount < minEventThreshold) {
                    truncatedSampleCount += childValue.SampleCount;
                    truncatedEventCount += childValue.EventCount;
                    continue;
                }
                children.push_back(child);
            }

            std::sort(children.begin(), children.end(), [&](ui32 a, ui32 b) {
                TStringBuf nameA = stringTable.Get(renderedNodes[a].NameId);
                TStringBuf nameB = stringTable.Get(renderedNodes[b].NameId);
                return nameA != nameB
                    ? nameA < nameB
                    : stringTable.Get(renderedNodes[a].FileId) < stringTable.Get(renderedNodes[b].FileId);
            });

            // "(truncated stack)" should be sorted alphabetically among siblings
            static constexpr TStringBuf truncatedStackName = "(truncated stack)";
            bool truncatedAdded = false;
            for (ui32 child : children) {
                if (!truncatedAdded && truncatedEventCount > 0 &&
                    truncatedStackName < stringTable.Get(renderedNodes[child].NameId))
                {
                    nextLevel.push_back({
                        TruncatedNodeMarker,
                        static_cast<i32>(levelIdx),
                        truncatedSampleCount,
                        truncatedEventCount,
                    });
                    truncatedAdded = true;
                }
                nextLevel.push_back({child, static_cast<i32>(levelIdx), 0, 0});
            }

            if (!truncatedAdded && truncatedEventCount > 0) {
                nextLevel.push_back({
                    TruncatedNodeMarker,
                    static_cast<i32>(levelIdx),
                    truncatedSampleCount,
                    truncatedEventCount,
                });
            }
        }

        writer.EndArray();
        currentLevel.swap(nextLevel);
    }

    writer.EndArray();

    WriteKey(writer, "stringTable");
    writer.StartArray();
    stringTable.ForEach([&writer](TStringBuf s) {
        writer.String(s.data(), s.size());
    });
    writer.EndArray();

    WriteKey(writer, "meta");
    writer.StartObject();
    WriteKey(writer, "version");
    writer.Int(2);
    WriteKey(writer, "eventType");
    const auto& metadata = profile.GetMetadata();
    if (metadata.default_sample_type() > 0) {
        writer.Uint(stringTable.Intern(profile.Strings().Get(metadata.default_sample_type()).View()));
    } else {
        writer.Uint(common.Samples);
    }
    WriteKey(writer, "frameType");
    writer.Uint(common.Function);
    writer.EndObject();

    writer.EndObject();

    out.Write(buffer.GetString(), buffer.GetSize());
}

} // namespace

////////////////////////////////////////////////////////////////////////////////

void RenderFlameGraphJson(
    const NProto::NProfile::Profile& profile,
    IOutputStream& out,
    const NProto::NProfile::RenderOptions& options
) {
    NProto::NProfile::MergeOptions mergeOptions;
    mergeOptions.set_strip_garbage_root_frames(true);
    mergeOptions.set_sanitize_thread_names(true);
    mergeOptions.set_ignore_source_locations(options.ignore_source_locations());

    NProto::NProfile::Profile merged;
    MergeProfiles({profile}, &merged, mergeOptions);

    TProfile mergedProfile{&merged};
    auto keyIds = TLabelKeyIds::Build(mergedProfile);
    auto trie = BuildFlameTrie(mergedProfile, keyIds, options);
    RenderTrieToJson(trie, mergedProfile, keyIds, out, options);
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NPerforator::NProfile
