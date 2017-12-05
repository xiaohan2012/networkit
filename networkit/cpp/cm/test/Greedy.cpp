#include "Greedy.h"  // test

#include <algorithm>
#include <utility>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../../graph/Graph.h"
#include "../Greedy.h"


namespace NetworKit {
  
  TEST_F(GreedyTest, testConstructor){
    // nc 0 contains 0...4
    // nc 1 contains 5
    CoreMaximization::Greedy greedy(G, G.numberOfNodes());

    ASSERT_THAT(greedy.core_, testing::ElementsAre(2, 2, 2, 2, 2, 1));

    ASSERT_EQ(greedy.node_rem(0), 2);
    ASSERT_EQ(greedy.node_rem(5), 1);
    
    ASSERT_THAT(greedy.bucket_1_[1],
    		testing::ContainerEq(std::unordered_set<index>({5})));
    ASSERT_THAT(greedy.bucket_1_[5],
    		testing::ContainerEq(std::unordered_set<index>({0, 1, 4})));

    ASSERT_THAT(greedy.nc_list_[0].usable,
    	        testing::ContainerEq(std::unordered_set<index>({0, 1, 4})));
    ASSERT_THAT(greedy.nc_list_[1].usable,
    		testing::ContainerEq(std::unordered_set<index>({5})));

    ASSERT_THAT(greedy.bucket_2_[1],
    		testing::ContainerEq(std::unordered_set<index>({1})));
    ASSERT_THAT(greedy.bucket_2_[5],
    		testing::ContainerEq(std::unordered_set<index>({0})));

    // cache ds
    ASSERT_TRUE(greedy.edge_gain_.empty());
    ASSERT_TRUE(greedy.added_edges_.empty());
    ASSERT_TRUE(greedy.evaluated_edges_.empty());
    ASSERT_TRUE(greedy.n2e_dep_.empty());
  }
}

