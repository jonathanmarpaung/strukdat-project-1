#include <gtest/gtest.h>
#include <vector>

#include "queue.h"

class QueueTest : public ::testing::Test {
protected:
  Queue q;

  void SetUp() override {
    initializeQueue(q);
  }

  void TearDown() override {
    while(!isQueueEmpty(q)) {
      QueueAddress node = dequeue(q);
      delete node;
    }
  }

  static QueueAddress makeNode() {
    QueueAddress node = new QueueElement;
    node->next = nullptr;
    node->prev = nullptr;
    return node;
  }

  void verifyQueueIntegrity(const std::vector<QueueAddress>& expected) const {
    if(expected.empty()) {
      EXPECT_TRUE(isQueueEmpty(q));
      EXPECT_EQ(q.head, nullptr);
      EXPECT_EQ(q.tail, nullptr);
      EXPECT_EQ(frontQueue(q), nullptr);
      EXPECT_EQ(sizeQueue(q), 0);
      return;
    }

    EXPECT_FALSE(isQueueEmpty(q));
    EXPECT_EQ(q.head, expected.front());
    EXPECT_EQ(q.tail, expected.back());
    EXPECT_EQ(frontQueue(q), expected.front());
    EXPECT_EQ(sizeQueue(q), static_cast<int>(expected.size()));

    // Cek ujung-ujung pointer DLL
    EXPECT_EQ(q.head->prev, nullptr) << "head->prev harus nullptr (terdeteksi dangling pointer!)";
    EXPECT_EQ(q.tail->next, nullptr) << "tail->next harus nullptr";

    // Traversal maju & cek tautan balik (prev)
    QueueAddress curr = q.head;
    for(size_t i = 0; i < expected.size(); ++i) {
      ASSERT_NE(curr, nullptr);
      EXPECT_EQ(curr, expected[i]);
      if(i > 0) {
        EXPECT_EQ(curr->prev, expected[i - 1]);
      }
      if(i + 1 < expected.size()) {
        EXPECT_EQ(curr->next, expected[i + 1]);
      }
      curr = curr->next;
    }
    EXPECT_EQ(curr, nullptr);
  }
};

// --- Initialization & Empty Operations ---

TEST_F(QueueTest, InitializeCreatesEmptyQueue) {
  verifyQueueIntegrity({});
}

TEST_F(QueueTest, DequeueOnEmptyQueueReturnsNullptr) {
  EXPECT_EQ(dequeue(q), nullptr);
  verifyQueueIntegrity({});
}

// --- Enqueue & FIFO Order ---

TEST_F(QueueTest, EnqueueSingleAndMultipleMaintainsFIFO) {
  QueueAddress n1 = makeNode();
  QueueAddress n2 = makeNode();
  QueueAddress n3 = makeNode();

  enqueue(q, n1);
  verifyQueueIntegrity({n1});

  enqueue(q, n2);
  verifyQueueIntegrity({n1, n2});

  enqueue(q, n3);
  verifyQueueIntegrity({n1, n2, n3});
}

TEST_F(QueueTest, FrontQueueDoesNotRemoveElement) {
  QueueAddress n1 = makeNode();
  QueueAddress n2 = makeNode();
  enqueue(q, n1);
  enqueue(q, n2);

  EXPECT_EQ(frontQueue(q), n1);
  EXPECT_EQ(frontQueue(q), n1);
  EXPECT_EQ(sizeQueue(q), 2);
}

// --- Dequeue & Pointer Hygiene (Bug-Hunting Tests) ---

TEST_F(QueueTest, DequeueSingleElementResetsHeadAndTail) {
  QueueAddress n1 = makeNode();
  enqueue(q, n1);

  QueueAddress out = dequeue(q);
  EXPECT_EQ(out, n1);
  EXPECT_EQ(out->next, nullptr);
  EXPECT_EQ(out->prev, nullptr);
  verifyQueueIntegrity({});
  delete out;
}

TEST_F(QueueTest, DequeueMultipleClearsDetachedNodeAndNewHeadPrev) {
  // BUG TARGET:
  // 1. Setelah dequeue n1, q.head->prev (milik n2) masih menunjuk ke n1 (dangling pointer!)
  // 2. n1->next masih menunjuk ke n2!
  QueueAddress n1 = makeNode();
  QueueAddress n2 = makeNode();
  QueueAddress n3 = makeNode();

  enqueue(q, n1);
  enqueue(q, n2);
  enqueue(q, n3);

  QueueAddress out1 = dequeue(q);
  ASSERT_EQ(out1, n1);
  EXPECT_EQ(out1->next, nullptr) << "Node hasil dequeue masih menyimpan pointer next lama!";
  EXPECT_EQ(out1->prev, nullptr);
  delete out1;  // Jika n2->prev masih ke n1, sekarang n2->prev adalah dangling pointer

  verifyQueueIntegrity({n2, n3});

  QueueAddress out2 = dequeue(q);
  ASSERT_EQ(out2, n2);
  EXPECT_EQ(out2->next, nullptr);
  EXPECT_EQ(out2->prev, nullptr);
  delete out2;

  verifyQueueIntegrity({n3});

  QueueAddress out3 = dequeue(q);
  ASSERT_EQ(out3, n3);
  delete out3;

  verifyQueueIntegrity({});
}

TEST_F(QueueTest, ReEnqueueDequeuedNodeDoesNotCreateCycle) {
  // BUG TARGET: Karena dequeue tidak mereset front->next = nullptr dan enqueue
  // tidak mereset new_queue->next = nullptr, re-enqueue menciptakan circular list!
  QueueAddress n1 = makeNode();
  QueueAddress n2 = makeNode();

  enqueue(q, n1);
  enqueue(q, n2);

  QueueAddress popped = dequeue(q);
  ASSERT_EQ(popped, n1);

  // Gunakan ASSERT agar tidak masuk infinite loop saat memanggil sizeQueue()
  ASSERT_EQ(popped->next, nullptr)
      << "Berhenti sebelum re-enqueue untuk mencegah infinite loop akibat next tidak di-reset!";

  enqueue(q, popped);
  verifyQueueIntegrity({n2, n1});
}

TEST_F(QueueTest, EnqueueWithDirtyPointersStillProducesCleanQueue) {
  // BUG TARGET: Jika node masuk ke enqueue membawa next/prev tidak null
  QueueAddress dummy = makeNode();
  QueueAddress n1 = makeNode();
  n1->next = dummy;
  n1->prev = dummy;

  enqueue(q, n1);
  EXPECT_EQ(n1->next, nullptr);
  EXPECT_EQ(n1->prev, nullptr);

  // Reset manual agar TearDown tidak ikut crash jika test gagal
  n1->next = nullptr;
  n1->prev = nullptr;
  delete dummy;
}

TEST_F(QueueTest, InterleavedEnqueueDequeueAndReuseAfterEmpty) {
  QueueAddress n1 = makeNode();
  QueueAddress n2 = makeNode();
  QueueAddress n3 = makeNode();

  enqueue(q, n1);
  QueueAddress out1 = dequeue(q);
  EXPECT_EQ(out1, n1);
  delete out1;
  EXPECT_TRUE(isQueueEmpty(q));

  // Isi lagi setelah sempat kosong
  enqueue(q, n2);
  enqueue(q, n3);
  EXPECT_EQ(sizeQueue(q), 2);
  EXPECT_EQ(frontQueue(q), n2);
}

TEST_F(QueueTest, StressTestFIFOOrdering) {
  const int N = 1000;
  std::vector<QueueAddress> nodes;
  nodes.reserve(N);

  for(int i = 0; i < N; ++i) {
    QueueAddress node = makeNode();
    nodes.push_back(node);
    enqueue(q, node);
  }
  EXPECT_EQ(sizeQueue(q), N);

  for(int i = 0; i < N; ++i) {
    QueueAddress out = dequeue(q);
    ASSERT_EQ(out, nodes[i]);
    delete out;
  }
  EXPECT_TRUE(isQueueEmpty(q));
  EXPECT_EQ(sizeQueue(q), 0);
}