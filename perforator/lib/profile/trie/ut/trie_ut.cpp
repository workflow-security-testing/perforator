#include <perforator/lib/profile/trie/trie.h>

#include <library/cpp/testing/gtest/gtest.h>

using namespace NPerforator::NProfile;

TEST(ProfileTrie, Basic) {
    TProfileTrie<int, i64> trie{0};

    EXPECT_EQ(trie.NodeCount(), 1u);
    EXPECT_EQ(trie.GetIdentity(0), 0);
    EXPECT_EQ(trie.GetParent(0), 0u);
    EXPECT_EQ(trie.GetFirstChild(0), 0u);
}

TEST(ProfileTrie, AddChildren) {
    TProfileTrie<int, i64> trie{0};

    ui32 child1 = trie.GetOrCreateChild(0, 1);
    ui32 child2 = trie.GetOrCreateChild(0, 2);
    ui32 child3 = trie.GetOrCreateChild(0, 3);

    EXPECT_EQ(trie.NodeCount(), 4u);
    EXPECT_EQ(trie.GetIdentity(child1), 1);
    EXPECT_EQ(trie.GetIdentity(child2), 2);
    EXPECT_EQ(trie.GetIdentity(child3), 3);

    EXPECT_EQ(trie.GetParent(child1), 0u);
    EXPECT_EQ(trie.GetParent(child2), 0u);
    EXPECT_EQ(trie.GetParent(child3), 0u);
}

TEST(ProfileTrie, GetOrCreateIsIdempotent) {
    TProfileTrie<int, i64> trie{0};

    ui32 first = trie.GetOrCreateChild(0, 42);
    ui32 second = trie.GetOrCreateChild(0, 42);

    EXPECT_EQ(first, second);
    EXPECT_EQ(trie.NodeCount(), 2u);
}

TEST(ProfileTrie, DeepTree) {
    TProfileTrie<int, i64> trie{0};

    ui32 node = 0;
    for (int i = 1; i <= 100; ++i) {
        node = trie.GetOrCreateChild(node, i);
    }

    EXPECT_EQ(trie.NodeCount(), 101u);

    // Verify path from leaf to root
    for (int i = 100; i >= 1; --i) {
        EXPECT_EQ(trie.GetIdentity(node), i);
        node = trie.GetParent(node);
    }
    EXPECT_EQ(node, 0u);
}

TEST(ProfileTrie, AddValue) {
    TProfileTrie<int, i64> trie{0};

    ui32 child = trie.GetOrCreateChild(0, 1);
    trie.AddValue(child, 10);
    trie.AddValue(child, 5);

    EXPECT_EQ(trie.GetValue(child), 15);
    EXPECT_EQ(trie.GetValue(0), 0);
}

TEST(ProfileTrie, SiblingTraversal) {
    TProfileTrie<int, i64> trie{0};

    trie.GetOrCreateChild(0, 1);
    trie.GetOrCreateChild(0, 2);
    trie.GetOrCreateChild(0, 3);

    // Collect all children via sibling links
    TVector<int> children;
    for (ui32 child = trie.GetFirstChild(0); child != 0; child = trie.GetNextSibling(child)) {
        children.push_back(trie.GetIdentity(child));
    }

    EXPECT_EQ(children.size(), 3u);
    // Children are added in reverse order (newest first)
    EXPECT_EQ(children[0], 3);
    EXPECT_EQ(children[1], 2);
    EXPECT_EQ(children[2], 1);
}

TEST(ProfileTrie, Finalize) {
    TProfileTrie<int, i64> trie{0};

    trie.GetOrCreateChild(0, 1);
    trie.GetOrCreateChild(0, 2);
    trie.Finalize();

    // After finalize, structure should still be accessible
    EXPECT_EQ(trie.NodeCount(), 3u);
    EXPECT_EQ(trie.GetFirstChild(0), 2u);
}
