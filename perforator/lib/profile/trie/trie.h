#pragma once

#include <library/cpp/containers/absl_flat_hash/flat_hash_map.h>

#include <util/generic/vector.h>

namespace NPerforator::NProfile {

// Generic trie with Struct-of-Arrays layout.
template <typename TNodeId, typename TValue>
class TProfileTrie {
public:
    explicit TProfileTrie(const TNodeId& rootIdentity) {
        Identity_.push_back(rootIdentity);
        Parent_.push_back(0);
        Values_.push_back({});
        FirstChild_.push_back(0);
        NextSibling_.push_back(0);
    }

    ui32 NodeCount() const { return Identity_.size(); }

    // Read-only accessors
    const TNodeId& GetIdentity(ui32 idx) const { return Identity_[idx]; }
    const TValue& GetValue(ui32 idx) const { return Values_[idx]; }
    ui32 GetParent(ui32 idx) const { return Parent_[idx]; }
    ui32 GetFirstChild(ui32 idx) const { return FirstChild_[idx]; }
    ui32 GetNextSibling(ui32 idx) const { return NextSibling_[idx]; }

    // Mutating methods
    ui32 GetOrCreateChild(ui32 parentIdx, const TNodeId& identity) {
        TEdgeKey key{parentIdx, identity};
        auto [it, inserted] = EdgeToNode_.try_emplace(key, 0);
        if (!inserted) {
            return it->second;
        }

        ui32 newIdx = Identity_.size();
        Identity_.push_back(identity);
        Parent_.push_back(parentIdx);
        Values_.push_back({});
        FirstChild_.push_back(0);

        ui32 oldFirstChild = FirstChild_[parentIdx];
        NextSibling_.push_back(oldFirstChild);
        FirstChild_[parentIdx] = newIdx;

        it->second = newIdx;
        return newIdx;
    }

    void AddValue(ui32 idx, const TValue& value) {
        Values_[idx] += value;
    }

    void Finalize() {
        EdgeToNode_.clear();
        absl::flat_hash_map<TEdgeKey, ui32>{}.swap(EdgeToNode_);
    }

private:
    TVector<TNodeId> Identity_;
    TVector<ui32> Parent_;
    TVector<ui32> FirstChild_;
    TVector<ui32> NextSibling_;
    TVector<TValue> Values_;

    struct TEdgeKey {
        ui32 ParentIdx;
        TNodeId Identity;
        bool operator==(const TEdgeKey& other) const = default;
        template <typename H>
        friend H AbslHashValue(H h, const TEdgeKey& key) {
            return H::combine(std::move(h), key.ParentIdx, key.Identity);
        }
    };
    absl::flat_hash_map<TEdgeKey, ui32> EdgeToNode_;
};

} // namespace NPerforator::NProfile
