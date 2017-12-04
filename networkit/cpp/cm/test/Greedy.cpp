#include "Greedy.h"  // test

#include <algorithm>
#include <utility>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../../graph/Graph.h"
#include "../Greedy.h"


namespace NetworKit {
  
  TEST_F(GreedyTest, testConstructor){
    CoreMaximization::Greedy greedy(G, G.numberOfNodes());

    ASSERT_THAT(greedy.core_, testing::ElementsAre(2, 2, 2, 2, 2, 1));

    ASSERT_EQ(greedy.node_rem(0), 2);
    ASSERT_EQ(greedy.node_rem(5), 1);
    
    ASSERT_THAT(greedy.bucket_1_[1],
    		testing::ContainerEq(std::unordered_set<index>({5})));
    ASSERT_THAT(greedy.bucket_1_[2],
    		testing::ContainerEq(std::unordered_set<index>({0, 1, 4})));

    ASSERT_THAT(greedy.nc_list_[0].usable,
    		testing::ContainerEq(std::unordered_set<index>({5})));
    ASSERT_THAT(greedy.nc_list_[1].usable,
    	        testing::ContainerEq(std::unordered_set<index>({0, 1, 4})));
  }
}

