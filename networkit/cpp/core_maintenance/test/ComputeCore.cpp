/*
 * ComputeCoreGTest.cpp
 *
 *  Created on: 01.06.2014
 *      Author: Klara Reichard (klara.reichard@gmail.com), Marvin Ritter (marvin.ritter@gmail.com)
 */

#ifndef NOGTEST


#include <algorithm>

#include "ComputeCore.h"
#include "../glist.h"
#include "../../graph/GraphBuilder.h"
#include "../../io/METISGraphReader.h"
#include "../../auxiliary/NumericTools.h"
#include "../../distance/DynBFS.h"
#include "../../auxiliary/Parallel.h"

namespace NetworKit {

INSTANTIATE_TEST_CASE_P(InstantiationName, ComputeCoreGTest, testing::Values(
                                                               std::make_tuple(false, false)));

bool ComputeCoreGTest::isWeighted() const {
	return std::get<0>(GetParam());
}
bool ComputeCoreGTest::isDirected() const {
	return std::get<1>(GetParam());
}

Graph ComputeCoreGTest::createGraph(count n) const {
	bool weighted, directed;
	std::tie(weighted, directed) = GetParam();
	Graph G(n, false, false);
	return G;
}

count ComputeCoreGTest::countSelfLoopsManually(const Graph &G) {
	count c = 0;
	G.parallelForEdges([&](node u, node v) {
		if (u == v) {
			#pragma omp atomic
			c += 1;
		}
	});
	return c;
}

void ComputeCoreGTest::SetUp() {
	/*
	 *    0
	 *   / \
	 *  /   \
	 * /     \
	 * 1 --- 2
	 * ^ \  /|
	 * |  \/ |
	 * | / \ |
	 * |/   \|
	 * 3 --- 4
	 *
	 */
	n_nodes = 5;
	n_edges = 8;

	G = createGraph(5);
	edges = {
	  {0, 1}, {0, 2}, {1, 2}, {1, 3}, {2, 4}, {3, 4}, {1, 4}, {2, 3}
	};
	for (auto& e : edges) {
		node u = e.first;
		node v = e.second;
		G.addEdge(u, v);
	}
}


/** CONSTRUCTORS **/

TEST_P(ComputeCoreGTest, testCoreNumberAndRemainingDegree) {
	Graph G = Graph(this->G, false, false);

	ASSERT_FALSE(G.isWeighted());
	ASSERT_FALSE(G.isDirected());
	ASSERT_EQ(this->G.numberOfNodes(), G.numberOfNodes());
	ASSERT_EQ(this->G.numberOfEdges(), G.numberOfEdges());

	// test core number
	core::GLIST glist(G.numberOfNodes());
	std::vector<index> core(G.numberOfNodes());
	glist.ComputeCore(G, true, core);
	std::vector<index> expected_cores{{2, 3, 3, 3, 3}};
	for(unsigned int i=0; i<expected_cores.size(); i++){
	  ASSERT_EQ(expected_cores[i], core[i]);
	}

	// test the remaining degrees
	std::vector<core::GLIST::ListNode> nodes = glist.get_nodes();
	std::vector<count> rems(G.numberOfNodes());
	for(index i=0; i<G.numberOfNodes(); i++){
	  rems[i] = nodes[i].rem;
	}
	sort(std::next(rems.begin()), rems.end(), std::greater<int>());
	
	ASSERT_THAT(rems, testing::ElementsAre(2, 3, 2, 1, 0));
	
}

  TEST_P(ComputeCoreGTest, testAll) {
    Graph G = Graph(this->G, false, false);
    int N = G.numberOfNodes();

    // get core
    core::GLIST glist(N);
    std::vector<index> core(N);
    glist.ComputeCore(G, true, core);

    // CoreGuidedBFS
    std::vector<core::GLIST::CoreComponent> nc_list(N);
    std::vector<index> nc_ids(N);
    glist.CoreGuidedBFS(G, core, nc_list, nc_ids);

    
    ASSERT_THAT(nc_list[0].nodes,
		testing::ContainerEq(std::unordered_set<node>({0})));

    ASSERT_THAT(nc_list[1].nodes,
		testing::ContainerEq(std::unordered_set<node>({1, 2, 3, 4}))); 


    ASSERT_THAT(nc_ids, testing::ElementsAre(0, 1, 1, 1, 1));

    // glist.Remove(0, 1, G, core);
    // std::cerr << "remove done" << std::endl;
    
    // FakeInsert
    // repeat the same process should return the same result
    // std::cerr << "fake inserting" << std::endl;
    for(index i=0; i<10; i++){
      std::vector<node> affected_nodes;
      // std::cerr << "round: " << i << std::endl;
      glist.FakeInsert(0, 4, G, core, nc_ids, affected_nodes);
      // std::cerr << "done" << std::endl;
      // ASSERT_EQ(new_nc_id, 1);
      ASSERT_THAT(affected_nodes, testing::ElementsAre(0));
      ASSERT_THAT(core, testing::ElementsAre(2, 3, 3, 3, 3));
    }

  }    
} /* namespace NetworKit */

#endif /*NOGTEST */
