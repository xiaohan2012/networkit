#include "CoreMaximization.h"

namespace NetworKit {
  void CoreMaximizationTest::SetUp(){
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
  }
}
