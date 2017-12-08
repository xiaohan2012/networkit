#ifndef NOGTEST

#include "gtest/gtest.h"
#include "../glist.h"
#include "../gadget/treap.h"

namespace NetworKit{
  TEST(GadgetTest, testTreap) {
    count n = 1000000;
    gadget::Treap tree(n);
    std::vector<int> roots(100, n);
    std::vector<int> counts(100, 0);
    std::vector<int> numbers(n);

    for (index i = 0; i < n; ++i) {
      numbers[i] = rand() % 100;
      ++counts[numbers[i]];
      tree.Insert(i, rand() % 2, roots[numbers[i]]);
    }
    for (index i = 0; i < 100; ++i) {
      if (roots[i] != n) {
    	tree.Check(roots[i]);
    	ASSERT_EQ(tree.Size(roots[i]), counts[i]);
      }
    }
    {
      for (index i = 1; i < 100; ++i) {
    	roots[0] = tree.Merge(roots[0], roots[i]);
      }
      tree.Check(roots[0]);
      ASSERT_EQ(tree.Size(roots[0]), n);
      // printf("tree size: %d\n", tree.Size(roots[0]));
    }

    for (index i = 0; i < n; ++i) {
      const index b = numbers[i];
      const index nb = rand() % 100;
      if (nb != b) {
    	--counts[b];
    	++counts[nb];
    	numbers[i] = nb;
    	tree.Delete(i, roots[b]);
    	tree.Insert(i, rand() % 2, roots[nb]);
      }
    }
    // for (index i = 0; i < 100; ++i) {
    //   if (roots[i] != n) {
    // 	// tree.Check(roots[i]);
    // 	ASSERT_EQ(tree.Size(roots[i]), counts[i]);
    //   }
    // }
    // for (index i = 0; i < n; ++i) {
    //   const index b = numbers[i];
    //   tree.Delete(i, roots[b]);
    //   --counts[b];
    // }
    // for (index i = 0; i < 100; ++i) {
    //   ASSERT_EQ(roots[i], n);
    // }
    // for (index i = 0; i < n; ++i) {
    //   tree.Insert(i, false, roots[0]);
    // }
    // ASSERT_EQ(tree.Size(roots[0]), n);
    // // for (index i = 0; i < n; ++i) {
    // //   ASSERT_EQ(tree.Rank(i, roots[0]), i + 1);
    // // }
    // for (index i = 0; i < n; ++i) {
    //   tree.Delete(i, roots[0]);
    //   tree.InsertAfter(i, (n - 1 + i) % n, roots[0]);
    // }
    // // for (index i = 0; i < n; ++i) {
    // //   ASSERT_EQ(tree.Rank(i, roots[0]), i + 1);
    // // }
    // // tree.Check(roots[0]);
  }
}

#endif /*NOGTEST */
