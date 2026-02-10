#include <perforator/lib/profile/trie/trie.h>

#include <library/cpp/testing/gtest/gtest.h>

using namespace NPerforator::NProfile;

TEST(TTrie, Basic) {
    TTrie<int, i64> trie;

    EXPECT_EQ(trie.NodeCount(), 1u);
    EXPECT_TRUE(trie.Root().GetFirstChild().IsZero());
}

TEST(TTrie, AddChildren) {
    TTrie<int, i64> trie;

    auto root = trie.Root();
    auto child1 = root.GetOrCreateChild(1);
    auto child2 = root.GetOrCreateChild(2);
    auto child3 = root.GetOrCreateChild(3);

    EXPECT_EQ(trie.NodeCount(), 4u);
    EXPECT_EQ(child1.GetKey(), 1);
    EXPECT_EQ(child2.GetKey(), 2);
    EXPECT_EQ(child3.GetKey(), 3);
}

TEST(TTrie, GetOrCreateIsIdempotent) {
    TTrie<int, i64> trie;

    auto root = trie.Root();
    auto first = root.GetOrCreateChild(42);
    auto second = root.GetOrCreateChild(42);

    EXPECT_EQ(first.GetId(), second.GetId());
    EXPECT_EQ(trie.NodeCount(), 2u);
}

TEST(TTrie, DeepTree) {
    TTrie<int, i64> trie;

    auto node = trie.Root();
    for (int i = 1; i <= 100; ++i) {
        node = node.GetOrCreateChild(i);
    }

    EXPECT_EQ(trie.NodeCount(), 101u);
    EXPECT_EQ(node.GetKey(), 100);
}

TEST(TTrie, AddValue) {
    TTrie<int, i64> trie;

    auto root = trie.Root();
    auto child = root.GetOrCreateChild(1);
    child.GetValue() += 10;
    child.GetValue() += 5;

    EXPECT_EQ(child.GetValue(), 15);
    EXPECT_EQ(root.GetValue(), 0);
}

TEST(TTrie, SiblingTraversal) {
    TTrie<int, i64> trie;

    auto root = trie.Root();
    root.GetOrCreateChild(1);
    root.GetOrCreateChild(2);
    root.GetOrCreateChild(3);

    // Collect all children via sibling links
    TVector<int> children;
    for (auto child = root.GetFirstChild(); !child.IsZero(); child = child.GetNextSibling()) {
        children.push_back(child.GetKey());
    }

    EXPECT_EQ(children.size(), 3u);
    // Children are added in reverse order (newest first)
    EXPECT_EQ(children[0], 3);
    EXPECT_EQ(children[1], 2);
    EXPECT_EQ(children[2], 1);
}

TEST(TTrie, Finalize) {
    TTrie<int, i64> trie;

    auto root = trie.Root();
    root.GetOrCreateChild(1);
    root.GetOrCreateChild(2);
    trie.Finalize();

    // After finalize, structure should still be accessible
    EXPECT_EQ(trie.NodeCount(), 3u);
    EXPECT_EQ(root.GetFirstChild().GetId(), 2u);
}
