#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "genre.h"

class GenreTest : public ::testing::Test {
protected:
  GenreList list;

  void SetUp() override {
    initializeGenreList(list);
  }

  void TearDown() override {
    destroyGenreList(list);
  }

  std::vector<std::string> toVector() const {
    std::vector<std::string> result;
    GenreAddress curr = list.first;
    while(curr != nullptr) {
      result.push_back(curr->info.name);
      curr = curr->next;
    }
    return result;
  }
};

// --- Initialization & Empty State ---

TEST_F(GenreTest, InitializeCreatesEmptyList) {
  EXPECT_EQ(list.first, nullptr);
  EXPECT_TRUE(isGenreListEmpty(list));
  EXPECT_EQ(searchGenre(list, "Rock"), nullptr);
  EXPECT_FALSE(doesGenreExist(list, "Rock"));
}

TEST_F(GenreTest, CreateGenreElementSetsFieldsProperly) {
  GenreAddress elem = createGenreElement("Jazz");
  ASSERT_NE(elem, nullptr);
  EXPECT_EQ(elem->info.name, "Jazz");
  EXPECT_EQ(elem->next, nullptr);
  delete elem;
}

// --- Insert Operations ---

TEST_F(GenreTest, InsertFirstOnEmptyAndNonEmptyList) {
  insertGenreFirst(list, createGenreElement("Pop"));
  insertGenreFirst(list, createGenreElement("Rock"));
  insertGenreFirst(list, createGenreElement("Jazz"));

  EXPECT_FALSE(isGenreListEmpty(list));
  EXPECT_EQ(toVector(), (std::vector<std::string>{"Jazz", "Rock", "Pop"}));
}

TEST_F(GenreTest, InsertLastOnEmptyAndNonEmptyList) {
  insertGenreLast(list, createGenreElement("Pop"));
  insertGenreLast(list, createGenreElement("Rock"));
  insertGenreLast(list, createGenreElement("Jazz"));

  EXPECT_EQ(toVector(), (std::vector<std::string>{"Pop", "Rock", "Jazz"}));
}

TEST_F(GenreTest, InsertAfterMiddleAndLastAndInvalidTarget) {
  GenreAddress g1 = createGenreElement("Pop");
  GenreAddress g2 = createGenreElement("Rock");
  insertGenreLast(list, g1);
  insertGenreLast(list, g2);

  // Sisip di tengah
  insertGenreAfter(list, g1, createGenreElement("Indie"));
  EXPECT_EQ(toVector(), (std::vector<std::string>{"Pop", "Indie", "Rock"}));

  // Sisip setelah elemen terakhir
  insertGenreAfter(list, g2, createGenreElement("Metal"));
  EXPECT_EQ(toVector(), (std::vector<std::string>{"Pop", "Indie", "Rock", "Metal"}));

  // Target tidak ada di list (node asing) dan nullptr
  GenreAddress foreign = createGenreElement("Foreign");
  GenreAddress unused1 = createGenreElement("Unused1");
  GenreAddress unused2 = createGenreElement("Unused2");

  insertGenreAfter(list, foreign, unused1);
  insertGenreAfter(list, nullptr, unused2);

  EXPECT_EQ(toVector(), (std::vector<std::string>{"Pop", "Indie", "Rock", "Metal"}));

  delete foreign;
  delete unused1;
  delete unused2;
}

// --- Add, Search & Case Sensitivity ---

TEST_F(GenreTest, AddGenreRejectsDuplicatesAndIsCaseSensitive) {
  addGenre(list, "Rock");
  addGenre(list, "Pop");
  addGenre(list, "Rock");  // Duplikat persis, harus diabaikan
  addGenre(list, "rock");  // Beda huruf besar/kecil

  EXPECT_EQ(toVector(), (std::vector<std::string>{"Rock", "Pop", "rock"}));
  EXPECT_TRUE(doesGenreExist(list, "Rock"));
  EXPECT_TRUE(doesGenreExist(list, "rock"));
  EXPECT_FALSE(doesGenreExist(list, "ROCK"));
}

TEST_F(GenreTest, AddGenreHandlesEmptyStringAndSpecialChars) {
  addGenre(list, "");
  addGenre(list, "R&B / Soul");
  EXPECT_TRUE(doesGenreExist(list, ""));
  EXPECT_TRUE(doesGenreExist(list, "R&B / Soul"));
}

// --- Low-level Remove Operations ---

TEST_F(GenreTest, RemoveFirstSingleAndMultipleElements) {
  addGenre(list, "A");
  addGenre(list, "B");

  GenreAddress out = nullptr;
  removeFirstGenre(list, out);
  ASSERT_NE(out, nullptr);
  EXPECT_EQ(out->info.name, "A");
  EXPECT_EQ(out->next, nullptr);
  delete out;

  out = nullptr;
  removeFirstGenre(list, out);
  ASSERT_NE(out, nullptr);
  EXPECT_EQ(out->info.name, "B");
  EXPECT_EQ(out->next, nullptr);
  EXPECT_TRUE(isGenreListEmpty(list));
  delete out;
}

TEST_F(GenreTest, RemoveLastSingleAndMultipleElements) {
  addGenre(list, "A");
  addGenre(list, "B");
  addGenre(list, "C");

  GenreAddress out = nullptr;
  removeLastGenre(list, out);
  ASSERT_NE(out, nullptr);
  EXPECT_EQ(out->info.name, "C");
  EXPECT_EQ(out->next, nullptr);
  delete out;

  EXPECT_EQ(toVector(), (std::vector<std::string>{"A", "B"}));

  // Hapus sampai 1 elemen lalu kosong
  removeLastGenre(list, out);
  delete out;
  removeLastGenre(list, out);
  ASSERT_NE(out, nullptr);
  EXPECT_EQ(out->info.name, "A");
  EXPECT_TRUE(isGenreListEmpty(list));
  delete out;
}

TEST_F(GenreTest, RemoveAfterMiddleLastAndEndOfList) {
  addGenre(list, "A");
  addGenre(list, "B");
  addGenre(list, "C");

  GenreAddress a = searchGenre(list, "A");
  GenreAddress c = searchGenre(list, "C");
  GenreAddress out = nullptr;

  // Hapus elemen tengah ("B")
  removeAfterGenre(list, a, out);
  ASSERT_NE(out, nullptr);
  EXPECT_EQ(out->info.name, "B");
  EXPECT_EQ(out->next, nullptr);
  delete out;
  EXPECT_EQ(toVector(), (std::vector<std::string>{"A", "C"}));

  // Hapus setelah elemen terakhir ("C") -> tidak boleh menghapus apa pun
  out = nullptr;
  removeAfterGenre(list, c, out);
  EXPECT_EQ(out, nullptr);
  EXPECT_EQ(toVector(), (std::vector<std::string>{"A", "C"}));
}

TEST_F(GenreTest, RemoveLowLevelOnEmptyListSetsOutToNullptr) {
  // Menguji apakah 'out' di-reset ke nullptr jika pemanggil tidak menginisialisasi
  GenreAddress dummy = createGenreElement("Dummy");
  GenreAddress out = dummy;

  removeFirstGenre(list, out);
  EXPECT_EQ(out, nullptr);

  out = dummy;
  removeLastGenre(list, out);
  EXPECT_EQ(out, nullptr);

  out = dummy;
  removeAfterGenre(list, nullptr, out);
  EXPECT_EQ(out, nullptr);

  delete dummy;
}

// --- High-level removeGenre (Bug-Hunting Tests) ---

TEST_F(GenreTest, RemoveGenreFirstMiddleLastAndSingle) {
  addGenre(list, "A");
  addGenre(list, "B");
  addGenre(list, "C");
  addGenre(list, "D");

  removeGenre(list, "B");  // Tengah
  EXPECT_EQ(toVector(), (std::vector<std::string>{"A", "C", "D"}));

  removeGenre(list, "A");  // Awal
  EXPECT_EQ(toVector(), (std::vector<std::string>{"C", "D"}));

  removeGenre(list, "D");  // Akhir
  EXPECT_EQ(toVector(), (std::vector<std::string>{"C"}));

  removeGenre(list, "C");  // Elemen tunggal
  EXPECT_TRUE(isGenreListEmpty(list));
}

TEST_F(GenreTest, RemoveGenreOnEmptyListDoesNotCrash) {
  // BUG TARGET: Saat ini current == list.first (nullptr == nullptr) -> delete temp uninitialized!
  removeGenre(list, "NonExistent");
  EXPECT_TRUE(isGenreListEmpty(list));
}

TEST_F(GenreTest, RemoveGenreNonExistentOnNonEmptyListDoesNotCrash) {
  // BUG TARGET: Saat ini current menjadi nullptr lalu mengakses current->next -> Segfault!
  addGenre(list, "Rock");
  addGenre(list, "Jazz");

  removeGenre(list, "Classical");
  EXPECT_EQ(toVector(), (std::vector<std::string>{"Rock", "Jazz"}));
}

// --- Dump, Destroy & Stress Test ---

TEST_F(GenreTest, DumpGenreListFormatting) {
  testing::internal::CaptureStdout();
  dumpGenreList(list);
  EXPECT_EQ(testing::internal::GetCapturedStdout(), "\n");

  addGenre(list, "Rock");
  addGenre(list, "Jazz");
  addGenre(list, "Pop");

  testing::internal::CaptureStdout();
  dumpGenreList(list);
  EXPECT_EQ(testing::internal::GetCapturedStdout(), "Rock, Jazz, Pop\n");
}

TEST_F(GenreTest, DestroyGenreListAndReuse) {
  addGenre(list, "A");
  addGenre(list, "B");
  destroyGenreList(list);
  EXPECT_TRUE(isGenreListEmpty(list));

  // Idempoten (destroy 2x) dan dapat dipakai ulang
  destroyGenreList(list);
  addGenre(list, "C");
  EXPECT_EQ(toVector(), (std::vector<std::string>{"C"}));
}

TEST_F(GenreTest, StressTestManyGenres) {
  const int N = 500;
  for(int i = 0; i < N; ++i) {
    addGenre(list, "Genre_" + std::to_string(i));
  }
  EXPECT_EQ(toVector().size(), static_cast<size_t>(N));
  EXPECT_TRUE(doesGenreExist(list, "Genre_0"));
  EXPECT_TRUE(doesGenreExist(list, "Genre_499"));

  for(int i = 0; i < N; i += 2) {
    removeGenre(list, "Genre_" + std::to_string(i));
  }
  EXPECT_EQ(toVector().size(), static_cast<size_t>(N / 2));
  EXPECT_FALSE(doesGenreExist(list, "Genre_0"));
  EXPECT_TRUE(doesGenreExist(list, "Genre_1"));
}