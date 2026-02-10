#pragma once

#include <library/cpp/containers/absl_flat_hash/flat_hash_map.h>

#include <util/generic/vector.h>

#include <concepts>

namespace NPerforator::NProfile {

// Generic trie with Struct-of-Arrays layout.
template <typename TKey, typename TValue, std::integral TIndex = ui32>
class TTrie {
private:
    struct TEdgeKey {
        TKey Key;
        TIndex ParentId;

        bool operator==(const TEdgeKey& other) const = default;

        template <typename H>
        friend H AbslHashValue(H h, const TEdgeKey& key) {
            return H::combine(std::move(h), key.ParentId, key.Key);
        }
    };

public:
    friend class TNode;

    class TNode {
    public:
        TNode(TTrie* trie, TIndex id)
            : Trie_{trie}
            , Id_{id}
        {}

        TIndex GetId() const {
            return Id_;
        }

        bool IsZero() const {
            return Id_ == 0;
        }

        const TKey& GetKey() const {
            return Trie_->Keys_[Id_];
        }

        const TValue& GetValue() const {
            return Trie_->Values_[Id_];
        }

        TValue& GetValue() {
            return Trie_->Values_[Id_];
        }

        TNode GetFirstChild() const {
            return {Trie_, Trie_->FirstChild_[Id_]};
        }

        TNode GetNextSibling() const {
            return {Trie_, Trie_->NextSibling_[Id_]};
        }

        TNode GetOrCreateChild(const TKey& key) {
            TEdgeKey edgeKey{key, Id_};
            auto [it, inserted] = Trie_->EdgeToNode_.try_emplace(edgeKey, 0);
            if (!inserted) {
                return {Trie_, it->second};
            }

            TIndex newId = Trie_->Keys_.size();
            Trie_->Keys_.push_back(key);
            Trie_->Values_.push_back({});
            Trie_->FirstChild_.push_back(0);

            TIndex oldFirstChild = Trie_->FirstChild_[Id_];
            Trie_->NextSibling_.push_back(oldFirstChild);
            Trie_->FirstChild_[Id_] = newId;

            it->second = newId;
            return TNode{Trie_, newId};
        }

    private:
        TTrie* Trie_;
        TIndex Id_;
    };

    TTrie()
        : Keys_{{}}
        , Values_{{}}
        , FirstChild_{0}
        , NextSibling_{0}
    {}

    TNode Root() {
        return {this, 0};
    }

    TNode Root() const {
        return {const_cast<TTrie*>(this), 0};
    }

    TNode NodeAt(TIndex idx) {
        return {this, idx};
    }

    TNode NodeAt(TIndex idx) const {
        return {const_cast<TTrie*>(this), idx};
    }

    TIndex NodeCount() const {
        return Keys_.size();
    }

    void Finalize() {
        decltype(EdgeToNode_){}.swap(EdgeToNode_);
    }

private:
    TVector<TKey> Keys_;
    TVector<TValue> Values_;
    TVector<TIndex> FirstChild_;
    TVector<TIndex> NextSibling_;
    absl::flat_hash_map<TEdgeKey, TIndex> EdgeToNode_;
};

} // namespace NPerforator::NProfile
