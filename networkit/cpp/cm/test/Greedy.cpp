#include "Greedy.h"  // test

#include <algorithm>
#include <utility>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdexcept>

#include "../../graph/Graph.h"
#include "../Greedy.h"


namespace NetworKit {
  
  TEST_F(GreedyTest, testConstructor){
    // nc 0 contains 0...4
    // nc 1 contains 5
    CoreMaximization::Greedy greedy(G, G.numberOfNodes());

    ASSERT_THAT(greedy.core_, testing::ElementsAre(2, 2, 2, 2, 2, 1));
    ASSERT_THAT(greedy.nc_ids_, testing::ElementsAre(0, 0, 0, 0, 0, 1));

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
    ASSERT_EQ(greedy.dummy_node, 6);
    ASSERT_EQ(greedy.core_max_, 2);
    ASSERT_EQ(greedy.gain_max_, 5);
  }

  TEST_F(GreedyTest, testGetCandidateEdge){
    // nc 0 contains 0...4
    // nc 1 contains 5
    CoreMaximization::Greedy greedy(G, G.numberOfNodes());
    Edge e = greedy.getCandidateEdge();

    ASSERT_EQ(e, Edge(5, greedy.dummy_node));
    greedy.evaluated_edges_.insert(e);

    std::unordered_set<Edge> inter_edges;    
    
    for(int i=0; i<3; i++){
      Edge e = greedy.getCandidateEdge();
      greedy.evaluated_edges_.insert(e);
      inter_edges.insert(e);
    }
    std::unordered_set<Edge> expected = {Edge(0, 3), Edge(0, 4), Edge(1, 4)};
    // for(auto e: inter_edges){
    //   std::cerr << e.u << "," << e.v << std::endl;
    // }
    ASSERT_THAT(inter_edges,
    		testing::ContainerEq(expected));

    // inter-core edges
    // for(int i=0; i<3; i++){
    //   Edge e = greedy.getCandidateEdge();
    //   greedy.evaluated_edges_.insert(e);
    //   edges.insert(e);
    // }   
    
    ASSERT_THROW(greedy.getCandidateEdge(), std::runtime_error);
    
  }

  TEST_F(GreedyTest, testBestEdge){
    CoreMaximization::Greedy greedy(G, G.numberOfNodes());
    Edge e = greedy.bestEdge();
    std::unordered_set<Edge> best_edges = {Edge(0, 4), Edge(0, 3)};
    ASSERT_TRUE(best_edges.find(e) != best_edges.end());
    ASSERT_EQ(greedy.edge_score_[Edge(0, 4)], 5);
    ASSERT_EQ(greedy.edge_score_[Edge(1, 4)], 4);
    ASSERT_EQ(greedy.edge_score_[Edge(5, 6)], 1);
  }

  TEST_F(GreedyTest, testMaintain){
    // setup
    CoreMaximization::Greedy greedy(G, G.numberOfNodes());
    Edge e (0, 3);    
    greedy.glist_.Insert(e.u, e.v, greedy.g_, greedy.core_);

    // real stuff
    greedy.maintain(e);

    // nc_list_
    // nodes
    // usable
    greedy.glist_.PrintNCList(greedy.nc_list_);
    
    ASSERT_THAT(greedy.nc_list_[0].nodes,
    		testing::ContainerEq(std::unordered_set<index>({0, 1, 2, 3})));
    ASSERT_THAT(greedy.nc_list_[0].usable,
    		testing::ContainerEq(std::unordered_set<index>({0})));
    ASSERT_THAT(greedy.nc_list_[1].nodes,
    		testing::ContainerEq(std::unordered_set<index>({4})));
    ASSERT_THAT(greedy.nc_list_[1].usable,
    		testing::ContainerEq(std::unordered_set<index>({4})));
    ASSERT_THAT(greedy.nc_list_[2].nodes,
    		testing::ContainerEq(std::unordered_set<index>({5})));
    ASSERT_THAT(greedy.nc_list_[2].usable,
    		testing::ContainerEq(std::unordered_set<index>({5})));

    // nc_ids_
    ASSERT_THAT(greedy.nc_ids_, testing::ElementsAre(2, 2, 2, 2, 0, 1));

    // now only inter-core edges exist
    // bucket_1_
    ASSERT_THAT(greedy.bucket_1_[1],
    		testing::ContainerEq(std::unordered_set<index>({4, 5})));
    for(index i=2; i<7; i++) {
      ASSERT_EQ(greedy.bucket_1_[i].size(), 0);  
    }    

    
    // bucket_2_
    ASSERT_THAT(greedy.bucket_2_[1],
    		testing::ContainerEq(std::unordered_set<index>({1})));
    for(index i=2; i<7; i++) {
      ASSERT_EQ(greedy.bucket_2_[i].size(), 0);  
    }


    ASSERT_EQ(greedy.current_score_, 1);
    ASSERT_EQ(greedy.core_max_, 3);
    ASSERT_EQ(greedy.gain_max_, 4);
  }
  TEST_F(GreedyTest, testDoGreedy){
    // CoreMaximization::Greedy greedy(G, G.numberOfNodes());
    // std::vector<Edge> edges = greedy.doGreedy(2);
    // ASSERT_EQ(edges[0], Edge(1, 4));
    // ASSERT_EQ(edges[1].u, 0);
  }  
}
