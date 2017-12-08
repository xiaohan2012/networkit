#ifndef NOGTEST

#include "FakeInsert.h"

namespace NetworKit {
  void FakeInsertTest::SetUp(){
    /*
     *    0
     *   / \
     *  /   \
     * /     \
     * 1 --- 2
     * |    /|
     * |   / |
     * | /   |
     * |/    |
     * 3 --- 4 --- 5
     *
     * node       : 0  1  2  3  4  5
     * core number: 2  2  2  2  2  1
     */    
    G = Graph(6, false, false);
    edges = {
      {0, 1}, {0, 2}, {1, 2}, {1, 3}, {2, 4}, {3, 4}, {2, 3}, {4, 5}
    };
    for (auto& e : edges) {
      node u = e.first;
      node v = e.second;
      G.addEdge(u, v);
    }

    int N = G.numberOfNodes();

    // get core
    glist_ptr = new core::GLIST(N);
    core  = std::vector<index>(N);
    glist_ptr->ComputeCore(G, true, core);

    // CoreGuidedBFS
    nc_list = std::vector<core::GLIST::CoreComponent>(N);
    nc_ids = std::vector<index> (N);
    glist_ptr->CoreGuidedBFS(G, core, nc_list, nc_ids);    
  }

  TEST_F(FakeInsertTest, testIfCreatesNewCoreComponent){
    for(index i=0; i<10; i++){
      std::vector<node> affected_nodes;
      // std::cerr << "round: " << i << std::endl;
      index new_nc_id = glist_ptr->FakeInsert(0, 3, G, core, nc_ids, affected_nodes);
      // std::cerr << "done" << std::endl;
 // ASSERT_EQ(new_nc_id, 1);
      ASSERT_THAT(affected_nodes, testing::ElementsAre(0, 1, 2, 3));
      ASSERT_THAT(core, testing::ElementsAre(2, 2, 2, 2, 2, 1));
    }            
  } 
} /* namespace */

#endif /*NOGTEST */
