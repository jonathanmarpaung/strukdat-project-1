#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "song.h"

class SongTest : public ::testing::Test {
protected:
  SongList list;

  void SetUp() override {
    initializeSongList(list);
  }

  void TearDown() override {
    destroySongList(list);
  }

  static int countNodes(SongAddress node) {
    if(node == nullptr) return 0;
    return 1 + countNodes(node->left) + countNodes(node->right);
  }

  static bool isValidBST(SongAddress node, const std::string* min_id, const std::string* max_id) {
    if(node == nullptr) return true;
    if(min_id != nullptr && node->info.id <= *min_id) return false;
    if(max_id != nullptr && node->info.id >= *max_id) return false;
    return isValidBST(node->left, min_id, &node->info.id) &&
           isValidBST(node->right, &node->info.id, max_id);
  }
};

// --- Initialization & Empty State ---

TEST_F(SongTest, InitializeCreatesEmptyTree) {
  EXPECT_EQ(list.node, nullptr);
  EXPECT_TRUE(isSongListEmpty(list));
  EXPECT_FALSE(doesSongExist(list, "S01"));
  EXPECT_EQ(searchSong(list, "S01"), nullptr);
  EXPECT_EQ(findMinSongNode(list.node), nullptr);
}

TEST_F(SongTest, CreateSongNodeInitializesAllFields) {
  SongAddress node = createSongNode("S01", "Bohemian Rhapsody", "Queen", 6);
  ASSERT_NE(node, nullptr);
  EXPECT_EQ(node->info.id, "S01");
  EXPECT_EQ(node->info.title, "Bohemian Rhapsody");
  EXPECT_EQ(node->info.artist, "Queen");
  EXPECT_EQ(node->info.duration, 6);
  EXPECT_EQ(node->left, nullptr);
  EXPECT_EQ(node->right, nullptr);
  delete node;
}

// --- Add & Search ---

TEST_F(SongTest, AddSongsBuildsValidBSTAndRejectsDuplicateID) {
  addSong(list, "S05", "Title 5", "Artist 5", 4);
  addSong(list, "S03", "Title 3", "Artist 3", 3);
  addSong(list, "S07", "Title 7", "Artist 7", 5);
  addSong(list, "S02", "Title 2", "Artist 2", 2);
  addSong(list, "S04", "Title 4", "Artist 4", 4);
  addSong(list, "S06", "Title 6", "Artist 6", 6);
  addSong(list, "S08", "Title 8", "Artist 8", 3);

  // Duplikat ID tidak boleh menimpa atau menambah node
  addSong(list, "S05", "Duplicate Title", "Other Artist", 99);

  EXPECT_EQ(countNodes(list.node), 7);
  EXPECT_TRUE(isValidBST(list.node, nullptr, nullptr));

  SongAddress found = searchSong(list, "S05");
  ASSERT_NE(found, nullptr);
  EXPECT_EQ(found->info.title, "Title 5");
  EXPECT_EQ(found->info.duration, 4);
}

TEST_F(SongTest, SkewedTreeAscendingAndDescending) {
  // Right-skewed tree
  addSong(list, "S01", "A", "X", 1);
  addSong(list, "S02", "B", "X", 2);
  addSong(list, "S03", "C", "X", 3);
  addSong(list, "S04", "D", "X", 4);

  EXPECT_EQ(countNodes(list.node), 4);
  EXPECT_TRUE(isValidBST(list.node, nullptr, nullptr));
  EXPECT_TRUE(doesSongExist(list, "S04"));

  destroySongList(list);

  // Left-skewed tree
  addSong(list, "S04", "D", "X", 4);
  addSong(list, "S03", "C", "X", 3);
  addSong(list, "S02", "B", "X", 2);
  addSong(list, "S01", "A", "X", 1);

  EXPECT_EQ(countNodes(list.node), 4);
  EXPECT_TRUE(isValidBST(list.node, nullptr, nullptr));
  EXPECT_TRUE(doesSongExist(list, "S01"));
}

// --- findMinSongNode (Bug-Hunting Test) ---

TEST_F(SongTest, FindMinSongNodeReturnsLeftmostNode) {
  // BUG TARGET: findMinSongNode saat ini me-return 'node' (root), bukan 'min_node'!
  addSong(list, "S05", "T5", "A5", 5);
  addSong(list, "S03", "T3", "A3", 3);
  addSong(list, "S01", "T1", "A1", 1);
  addSong(list, "S04", "T4", "A4", 4);
  addSong(list, "S08", "T8", "A8", 8);

  SongAddress min_node = findMinSongNode(list.node);
  ASSERT_NE(min_node, nullptr);
  EXPECT_EQ(min_node->info.id, "S01");
}

// --- Delete Operations (All BST Cases) ---

TEST_F(SongTest, DeleteFromEmptyTreeAndNonExistentID) {
  deleteSong(list, "S99");
  EXPECT_TRUE(isSongListEmpty(list));

  addSong(list, "S05", "T5", "A5", 5);
  deleteSong(list, "S99");
  EXPECT_EQ(countNodes(list.node), 1);
  EXPECT_TRUE(doesSongExist(list, "S05"));
}

TEST_F(SongTest, DeleteRootWhenSingleNode) {
  addSong(list, "S05", "T5", "A5", 5);
  deleteSong(list, "S05");
  EXPECT_TRUE(isSongListEmpty(list));
  EXPECT_EQ(list.node, nullptr);
}

TEST_F(SongTest, DeleteLeafNode) {
  addSong(list, "S05", "T5", "A5", 5);
  addSong(list, "S03", "T3", "A3", 3);
  addSong(list, "S07", "T7", "A7", 7);

  deleteSong(list, "S03");
  EXPECT_FALSE(doesSongExist(list, "S03"));
  EXPECT_EQ(countNodes(list.node), 2);
  EXPECT_TRUE(isValidBST(list.node, nullptr, nullptr));
}

TEST_F(SongTest, DeleteNodeWithOnlyLeftChild) {
  addSong(list, "S05", "T5", "A5", 5);
  addSong(list, "S03", "T3", "A3", 3);
  addSong(list, "S02", "T2", "A2", 2);

  deleteSong(list, "S03");
  EXPECT_FALSE(doesSongExist(list, "S03"));
  EXPECT_TRUE(doesSongExist(list, "S02"));
  EXPECT_EQ(countNodes(list.node), 2);
  EXPECT_TRUE(isValidBST(list.node, nullptr, nullptr));
}

TEST_F(SongTest, DeleteNodeWithOnlyRightChild) {
  addSong(list, "S05", "T5", "A5", 5);
  addSong(list, "S07", "T7", "A7", 7);
  addSong(list, "S09", "T9", "A9", 9);

  deleteSong(list, "S07");
  EXPECT_FALSE(doesSongExist(list, "S07"));
  EXPECT_TRUE(doesSongExist(list, "S09"));
  EXPECT_EQ(countNodes(list.node), 2);
  EXPECT_TRUE(isValidBST(list.node, nullptr, nullptr));
}

TEST_F(SongTest, DeleteNodeWithTwoChildrenDirectRightSuccessor) {
  // BUG TARGET: deleteSongNode memanggil deleteSongNode(node->right, id) alih-alih temp->info.id
  // sehingga S07 menjadi duplikat dan jumlah node tetap 3!
  addSong(list, "S05", "T5", "A5", 5);
  addSong(list, "S03", "T3", "A3", 3);
  addSong(list, "S07", "T7", "A7", 7);

  deleteSong(list, "S05");

  EXPECT_FALSE(doesSongExist(list, "S05"));
  EXPECT_TRUE(doesSongExist(list, "S03"));
  EXPECT_TRUE(doesSongExist(list, "S07"));
  EXPECT_EQ(countNodes(list.node), 2) << "Successor tidak terhapus dari right subtree (duplikat)!";
  EXPECT_TRUE(isValidBST(list.node, nullptr, nullptr));
}

TEST_F(SongTest, DeleteNodeWithTwoChildrenDeepSuccessor) {
  // BUG TARGET: Menguji kombinasi bug findMinSongNode + salah argumen id pada deleteSongNode.
  // Tree:
  //        S05
  //       /   \
  //     S02   S08
  //           / \
  //         S06 S09
  //           \
  //           S07
  // Successor yang benar untuk S05 adalah S06 (bukan S08), dan S07 harus tetap terjangkau.
  addSong(list, "S05", "T5", "A5", 5);
  addSong(list, "S02", "T2", "A2", 2);
  addSong(list, "S08", "T8", "A8", 8);
  addSong(list, "S06", "T6", "A6", 6);
  addSong(list, "S09", "T9", "A9", 9);
  addSong(list, "S07", "T7", "A7", 7);

  deleteSong(list, "S05");

  EXPECT_FALSE(doesSongExist(list, "S05"));
  EXPECT_EQ(countNodes(list.node), 5);
  EXPECT_TRUE(isValidBST(list.node, nullptr, nullptr))
      << "BST invariant rusak setelah menghapus node dengan 2 child!";

  for(const auto& id : {"S02", "S06", "S07", "S08", "S09"}) {
    EXPECT_TRUE(doesSongExist(list, id)) << "Node " << id << " hilang / tidak dapat dicari!";
  }
}

// --- Update Operations ---

TEST_F(SongTest, UpdateArtistTitleDurationOnExistingAndMissingSong) {
  addSong(list, "S01", "Old Title", "Old Artist", 3);

  updateSongTitle(list, "S01", "New Title");
  updateSongArtist(list, "S01", "New Artist");
  updateSongDuration(list, "S01", 5);

  SongAddress s = searchSong(list, "S01");
  ASSERT_NE(s, nullptr);
  EXPECT_EQ(s->info.title, "New Title");
  EXPECT_EQ(s->info.artist, "New Artist");
  EXPECT_EQ(s->info.duration, 5);

  // Update pada ID yang tidak ada harus aman (no-op)
  updateSongTitle(list, "MISSING", "X");
  updateSongArtist(list, "MISSING", "Y");
  updateSongDuration(list, "MISSING", 10);
  EXPECT_FALSE(doesSongExist(list, "MISSING"));
}

// --- Dump & Destroy ---

TEST_F(SongTest, DumpSongListOutputsSongDetails) {
  addSong(list, "S01", "Lagu Satu", "Artis A", 4);

  testing::internal::CaptureStdout();
  dumpSongList(list);
  std::string output = testing::internal::GetCapturedStdout();

  EXPECT_NE(output.find("ID: S01"), std::string::npos);
  EXPECT_NE(output.find("Title: Lagu Satu"), std::string::npos);
  EXPECT_NE(output.find("Artist: Artis A"), std::string::npos);
  EXPECT_NE(output.find("Duration: 4 minutes"), std::string::npos);
}

TEST_F(SongTest, DestroySongListClearsAndAllowsReuse) {
  addSong(list, "S05", "T5", "A5", 5);
  addSong(list, "S02", "T2", "A2", 2);
  addSong(list, "S08", "T8", "A8", 8);

  destroySongList(list);
  EXPECT_TRUE(isSongListEmpty(list));
  EXPECT_EQ(list.node, nullptr);

  // Destroy dua kali harus aman
  destroySongList(list);

  // Dapat diisi ulang
  addSong(list, "S10", "T10", "A10", 3);
  EXPECT_TRUE(doesSongExist(list, "S10"));
  EXPECT_EQ(countNodes(list.node), 1);
}