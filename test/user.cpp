#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "user.h"

class UserTest : public ::testing::Test {
protected:
  UserList list;

  void SetUp() override {
    initializeUserList(list);
  }

  void TearDown() override {
    // Cleanup manual menggunakan loop agar tidak bergantung pada bug if vs while di destroyUserList
    while(!isUserListEmpty(list)) {
      UserAddress curr = list.first;
      list.first = curr->next;
      if(list.first == nullptr) {
        list.last = nullptr;
      }
      delete curr;
    }
  }

  // Memastikan integritas Double Linked List (maju & mundur)
  void verifyDLLIntegrity(const std::vector<std::string>& expected_usernames) const {
    if(expected_usernames.empty()) {
      EXPECT_TRUE(isUserListEmpty(list));
      EXPECT_EQ(list.first, nullptr);
      EXPECT_EQ(list.last, nullptr);
      return;
    }

    EXPECT_FALSE(isUserListEmpty(list));
    ASSERT_NE(list.first, nullptr);
    ASSERT_NE(list.last, nullptr);

    EXPECT_EQ(list.first->prev, nullptr) << "list.first->prev harus nullptr!";
    EXPECT_EQ(list.last->next, nullptr) << "list.last->next harus nullptr!";

    // Traversal maju (first -> last)
    std::vector<std::string> forward;
    UserAddress curr = list.first;
    UserAddress prev = nullptr;
    while(curr != nullptr) {
      EXPECT_EQ(curr->prev, prev)
          << "Link prev rusak pada user '" << curr->info.username << "'!";
      forward.push_back(curr->info.username);
      prev = curr;
      curr = curr->next;
    }
    EXPECT_EQ(prev, list.last);
    EXPECT_EQ(forward, expected_usernames);

    // Traversal mundur (last -> first)
    std::vector<std::string> backward;
    curr = list.last;
    while(curr != nullptr) {
      backward.push_back(curr->info.username);
      curr = curr->prev;
    }
    std::vector<std::string> expected_rev(expected_usernames.rbegin(), expected_usernames.rend());
    EXPECT_EQ(backward, expected_rev);
  }
};

// --- Initialization & Empty State ---

TEST_F(UserTest, InitializeCreatesEmptyList) {
  verifyDLLIntegrity({});
  EXPECT_EQ(searchUser(list, "alice"), nullptr);
  EXPECT_FALSE(doesUserExist(list, "alice"));
}

TEST_F(UserTest, CreateUserElementSetsFieldsProperly) {
  UserAddress u = createUserElement("alice", "Alice", "premium");
  ASSERT_NE(u, nullptr);
  EXPECT_EQ(u->info.username, "alice");
  EXPECT_EQ(u->info.name, "Alice");
  EXPECT_EQ(u->info.type, "premium");
  EXPECT_EQ(u->next, nullptr);
  EXPECT_EQ(u->prev, nullptr);
  delete u;
}

// --- Insert Operations ---

TEST_F(UserTest, InsertFirstAndLastMaintainDLLLinks) {
  insertUserFirst(list, createUserElement("u2", "User 2", "free"));
  verifyDLLIntegrity({"u2"});

  insertUserFirst(list, createUserElement("u1", "User 1", "free"));
  verifyDLLIntegrity({"u1", "u2"});

  insertUserLast(list, createUserElement("u3", "User 3", "free"));
  verifyDLLIntegrity({"u1", "u2", "u3"});
}

TEST_F(UserTest, InsertAfterMiddleLastAndNullTarget) {
  UserAddress u1 = createUserElement("u1", "User 1", "free");
  UserAddress u3 = createUserElement("u3", "User 3", "free");
  insertUserLast(list, u1);
  insertUserLast(list, u3);

  // Sisip di tengah
  insertUserAfter(list, u1, createUserElement("u2", "User 2", "free"));
  verifyDLLIntegrity({"u1", "u2", "u3"});

  // Sisip setelah last
  insertUserAfter(list, u3, createUserElement("u4", "User 4", "free"));
  verifyDLLIntegrity({"u1", "u2", "u3", "u4"});

  // Target nullptr harus diabaikan
  UserAddress unused = createUserElement("x", "X", "free");
  insertUserAfter(list, nullptr, unused);
  verifyDLLIntegrity({"u1", "u2", "u3", "u4"});
  delete unused;
}

// --- Register & Profile Updates ---

TEST_F(UserTest, RegisterUserDefaultsToFreeAndRejectsDuplicate) {
  registerUser(list, "alice", "Alice A");
  registerUser(list, "bob", "Bob B");
  registerUser(list, "alice", "Alice Impostor");  // Duplikat

  verifyDLLIntegrity({"alice", "bob"});

  UserAddress alice = searchUser(list, "alice");
  ASSERT_NE(alice, nullptr);
  EXPECT_EQ(alice->info.name, "Alice A");
  EXPECT_EQ(alice->info.type, "free");
}

TEST_F(UserTest, SetUserTypeValidInvalidAndNonExistent) {
  registerUser(list, "alice", "Alice");

  setUserType(list, "alice", "premium");
  EXPECT_EQ(searchUser(list, "alice")->info.type, "premium");

  // Tipe tidak valid tidak boleh mengubah nilai
  setUserType(list, "alice", "vip");
  setUserType(list, "alice", "PREMIUM");
  setUserType(list, "alice", "");
  EXPECT_EQ(searchUser(list, "alice")->info.type, "premium");

  setUserType(list, "alice", "free");
  EXPECT_EQ(searchUser(list, "alice")->info.type, "free");

  // User tidak ada
  setUserType(list, "ghost", "premium");
  EXPECT_FALSE(doesUserExist(list, "ghost"));
}

TEST_F(UserTest, SetUserNameExistingAndNonExistent) {
  registerUser(list, "alice", "Alice Old");
  setUserName(list, "alice", "Alice New");
  EXPECT_EQ(searchUser(list, "alice")->info.name, "Alice New");

  setUserName(list, "ghost", "Ghost Name");
  EXPECT_FALSE(doesUserExist(list, "ghost"));
}

// --- Remove Operations (Bug-Hunting Tests) ---

TEST_F(UserTest, RemoveFirstClearsNewFirstPrevPointer) {
  // BUG TARGET: removeFirstUser tidak mereset list.first->prev = nullptr!
  registerUser(list, "u1", "User 1");
  registerUser(list, "u2", "User 2");
  registerUser(list, "u3", "User 3");

  UserAddress out = nullptr;
  removeFirstUser(list, out);
  ASSERT_NE(out, nullptr);
  EXPECT_EQ(out->info.username, "u1");
  EXPECT_EQ(out->next, nullptr);
  EXPECT_EQ(out->prev, nullptr);
  delete out;

  verifyDLLIntegrity({"u2", "u3"});
}

TEST_F(UserTest, RemoveLastSingleAndMultipleMaintainsLinks) {
  registerUser(list, "u1", "User 1");
  registerUser(list, "u2", "User 2");

  UserAddress out = nullptr;
  removeLastUser(list, out);
  ASSERT_NE(out, nullptr);
  EXPECT_EQ(out->info.username, "u2");
  EXPECT_EQ(out->next, nullptr);
  EXPECT_EQ(out->prev, nullptr);
  delete out;
  verifyDLLIntegrity({"u1"});

  removeLastUser(list, out);
  ASSERT_NE(out, nullptr);
  EXPECT_EQ(out->info.username, "u1");
  delete out;
  verifyDLLIntegrity({});
}

TEST_F(UserTest, RemoveAfterMiddleFixesNextNodePrevPointer) {
  // BUG TARGET: removeAfterUser saat menghapus node tengah tidak memperbarui
  // target->next->prev = target, sehingga u3->prev masih menunjuk ke u2 yang sudah dihapus!
  registerUser(list, "u1", "User 1");
  registerUser(list, "u2", "User 2");
  registerUser(list, "u3", "User 3");

  UserAddress u1 = searchUser(list, "u1");
  UserAddress out = nullptr;

  removeAfterUser(list, u1, out);
  ASSERT_NE(out, nullptr);
  EXPECT_EQ(out->info.username, "u2");
  EXPECT_EQ(out->next, nullptr);
  EXPECT_EQ(out->prev, nullptr);
  delete out;

  verifyDLLIntegrity({"u1", "u3"});
}

TEST_F(UserTest, RemoveAfterTargetingLastNodeUpdatesListLast) {
  registerUser(list, "u1", "User 1");
  registerUser(list, "u2", "User 2");

  UserAddress u1 = searchUser(list, "u1");
  UserAddress out = nullptr;

  removeAfterUser(list, u1, out);
  ASSERT_NE(out, nullptr);
  EXPECT_EQ(out->info.username, "u2");
  delete out;

  verifyDLLIntegrity({"u1"});
}

TEST_F(UserTest, RemoveLowLevelOnEmptyListSetsOutToNullptr) {
  UserAddress dummy = createUserElement("d", "D", "free");
  UserAddress out = dummy;

  removeFirstUser(list, out);
  EXPECT_EQ(out, nullptr);

  out = dummy;
  removeLastUser(list, out);
  EXPECT_EQ(out, nullptr);

  out = dummy;
  removeAfterUser(list, nullptr, out);
  EXPECT_EQ(out, nullptr);

  delete dummy;
}

// --- Deactivate User Sequences (Bug-Hunting Tests) ---

TEST_F(UserTest, DeactivateAdjacentMiddleUsersSafely) {
  // BUG TARGET: Menghapus "u2" merusak u3->prev (dangling pointer).
  // Jika kemudian "u3" dihapus via deactivateUser(list, "u3"), fungsi memanggil
  // removeAfterUser(list, u3->prev, temp) dengan pointer use-after-free!
  registerUser(list, "u1", "User 1");
  registerUser(list, "u2", "User 2");
  registerUser(list, "u3", "User 3");
  registerUser(list, "u4", "User 4");

  deactivateUser(list, "u2");
  // Cek integritas sebelum lanjut agar gagal dengan jelas (bukan UB)
  UserAddress u1 = searchUser(list, "u1");
  UserAddress u3 = searchUser(list, "u3");
  ASSERT_EQ(u3->prev, u1) << "u3->prev rusak (dangling pointer ke u2 yang sudah di-delete)!";

  deactivateUser(list, "u3");
  verifyDLLIntegrity({"u1", "u4"});

  deactivateUser(list, "u1");
  verifyDLLIntegrity({"u4"});

  deactivateUser(list, "u4");
  verifyDLLIntegrity({});
}

TEST_F(UserTest, DeactivateNonExistentAndEmptyListIsSafe) {
  deactivateUser(list, "nobody");
  verifyDLLIntegrity({});

  registerUser(list, "u1", "User 1");
  deactivateUser(list, "nobody");
  verifyDLLIntegrity({"u1"});
}

// --- Dump & Destroy ---

TEST_F(UserTest, DumpUserListOutputsAllUsers) {
  registerUser(list, "alice", "Alice");
  setUserType(list, "alice", "premium");
  registerUser(list, "bob", "Bob");

  testing::internal::CaptureStdout();
  dumpUserList(list);
  std::string output = testing::internal::GetCapturedStdout();

  EXPECT_NE(output.find("Username: alice"), std::string::npos);
  EXPECT_NE(output.find("Account Type: premium"), std::string::npos);
  EXPECT_NE(output.find("Username: bob"), std::string::npos);
  EXPECT_NE(output.find("Account Type: free"), std::string::npos);
}

TEST_F(UserTest, DestroyUserListFreesAllNodes) {
  // BUG TARGET: destroyUserList memakai 'if' bukan 'while'.
  // Jalankan test ini dengan -DENABLE_SANITIZERS=ON (ASan/LSan) untuk langsung
  // melihat laporan memory leak pada u2 dan u3!
  registerUser(list, "u1", "User 1");
  registerUser(list, "u2", "User 2");
  registerUser(list, "u3", "User 3");

  destroyUserList(list);
  verifyDLLIntegrity({});

  // Reuse setelah destroy
  registerUser(list, "u4", "User 4");
  verifyDLLIntegrity({"u4"});
}