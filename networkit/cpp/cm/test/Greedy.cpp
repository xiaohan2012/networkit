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

    ASSERT_EQ(greedy.bucket_2_[0].size(), 0);
    ASSERT_EQ(greedy.bucket_2_[1].size(), 0);
    for(int i=0; i<5; i++)
      ASSERT_EQ(greedy.bucket_2_[i].size(), 0);
    // testing::ContainerEq(std::unordered_set<index>({})));
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
    
    for(int i=0; i<4; i++){
      Edge e = greedy.getCandidateEdge();
      greedy.proposed_edges_.insert(e);
      inter_edges.insert(e);
    }
    std::unordered_set<Edge> expected = {Edge(5, 6), Edge(0, 3), Edge(0, 4), Edge(1, 4)};
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
    std::vector<node> affected_nodes;

    std::cerr << "glist.insert" << std::endl;
    greedy.glist_.Insert(e.u, e.v, greedy.g_, greedy.core_, affected_nodes);

    // real stuff
    // std::cerr << "before maintain" << std::endl;
    // greedy.glist_.PrintNCList(greedy.nc_list_);
    greedy.maintain(e, affected_nodes);

    // nc_list_
    // nodes
    // usable
    // std::cerr << "after maintain" << std::endl;    
    // greedy.glist_.PrintNCList(greedy.nc_list_);
    
    ASSERT_THAT(greedy.nc_list_[2].nodes,
    		testing::ContainerEq(std::unordered_set<index>({0, 1, 2, 3})));
    ASSERT_THAT(greedy.nc_list_[2].usable,
    		testing::ContainerEq(std::unordered_set<index>({0})));
    ASSERT_THAT(greedy.nc_list_[0].nodes,
    		testing::ContainerEq(std::unordered_set<index>({4})));
    ASSERT_THAT(greedy.nc_list_[0].usable,
    		testing::ContainerEq(std::unordered_set<index>({4})));
    ASSERT_THAT(greedy.nc_list_[1].nodes,
    		testing::ContainerEq(std::unordered_set<index>({5})));
    ASSERT_THAT(greedy.nc_list_[1].usable,
    		testing::ContainerEq(std::unordered_set<index>({5})));

    // nc_ids_
    ASSERT_THAT(greedy.nc_ids_, testing::ElementsAre(2, 2, 2, 2, 0, 1));

    // now only inter-core edges exist
    // bucket_1_
    ASSERT_THAT(greedy.bucket_1_[1],
    		testing::ContainerEq(std::unordered_set<index>({4, 5})));

    // note that {0, 1, 2, 3} should not have any candidate inter-core edges
    // beucase it's the highest core
    for(index i=2; i<7; i++) 
      ASSERT_EQ(greedy.bucket_1_[i].size(), 0);
    
    // bucket_2_
    // there are no intra-nc edges available
    for(index i=0; i<7; i++)
      if(i != 4)
	ASSERT_EQ(greedy.bucket_2_[i].size(), 0);

    // Strictly speaking, it should equal {} because nc 2 is a clique   
    ASSERT_THAT(greedy.bucket_2_[4],
		testing::ContainerEq(std::unordered_set<index>({2})));    

    ASSERT_EQ(greedy.current_score_, 0); // because no other edges are evaluated
    ASSERT_EQ(greedy.core_max_, 3);
    ASSERT_EQ(greedy.gain_max_, 4); // Strictly speaking, it should equal 1 because the largest nc is a clique
  }

  TEST_F(GreedyTest, testDoGreedySingleEdge){
    CoreMaximization::Greedy greedy(G, G.numberOfNodes());
    std::vector<Edge> edges = greedy.doGreedy(1);

    ASSERT_EQ(edges[0], Edge(0, 4));
    ASSERT_THAT(greedy.core_, testing::ElementsAre(3, 3, 3, 3, 3, 1));

    ASSERT_EQ(greedy.bucket_1_[0].size(), 0);
    ASSERT_THAT(greedy.nc_list_[1].nodes,
		testing::ContainerEq(std::unordered_set<index>({5})));
    ASSERT_THAT(greedy.nc_list_[2].nodes,
		testing::ContainerEq(std::unordered_set<index>({0, 1, 2, 3, 4})));
    ASSERT_THAT(greedy.nc_list_[2].usable,
		testing::ContainerEq(std::unordered_set<index>({0})));
    
    for(auto i: std::vector<int>({0, 2, 3, 4, 6}))
      ASSERT_EQ(greedy.bucket_1_[i].size(), 0);

    ASSERT_THAT(greedy.bucket_1_[1],
    		testing::ContainerEq(std::unordered_set<index>({5})));
    // ASSERT_THAT(greedy.bucket_1_[5],
    // 		testing::ContainerEq(std::unordered_set<index>({0}))); // one of {0, 1, 3}

    for(int i=0; i<7; i++)
      if(i != 1)
	ASSERT_EQ(greedy.bucket_1_[i].size(), 0);
    
    ASSERT_THAT(greedy.bucket_2_[5],
    		testing::ContainerEq(std::unordered_set<index>({2})));  // nc 0 moved to nc 2
    

    // check incremental data structures
    // BREAKPOINT
    // (0, 3) should be removed now
    ASSERT_THAT(greedy.evaluated_edges_,
    		testing::ContainerEq(std::unordered_set<Edge>({Edge(5, 6)})));

    // added (0, 4) is not in the dependency list
    for(node n=0; n<5; n++)
      ASSERT_TRUE(greedy.n2e_dep_[n].find(Edge(0, 4)) == greedy.n2e_dep_[n].end());

    ASSERT_EQ(greedy.current_score_, 1);
    ASSERT_EQ(greedy.gain_max_, 5);
  }

  TEST_F(GreedyTest, testDoGreedyTwoEdges){
    CoreMaximization::Greedy greedy(G, G.numberOfNodes());
    std::vector<Edge> edges = greedy.doGreedy(2);
    ASSERT_EQ(edges.size(), 2);
    ASSERT_EQ(edges[0], Edge(0, 4));
    ASSERT_EQ(edges[1].u, 5);

    std::unordered_set<node> execpted_nodes({0, 1, 2, 3, 4});
    ASSERT_TRUE(execpted_nodes.find(edges[1].v) != execpted_nodes.end());
  }

  TEST_F(GreedyTest, testDoGreedyUntilFullyConnected){
    CoreMaximization::Greedy greedy(G, G.numberOfNodes());
    std::vector<Edge> edges = greedy.doGreedy(7); // 7 more edges to make it clique
    ASSERT_THAT(greedy.core_, testing::ElementsAre(5, 5, 5, 5, 5, 5));
    // ASSERT_EQ(edges.size(), 3);
    // ASSERT_EQ(edges[2], Edge());
  }
}

