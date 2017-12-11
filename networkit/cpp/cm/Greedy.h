#ifndef CM_GREEDY_H_
#define CM_GREEDY_H_

#include <vector>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <iterator>


#include "../graph/Graph.h"
#include "../core_maintenance/glist.h"

namespace NetworKit{
  namespace CoreMaximization{
    class Greedy final {
    public:
      explicit Greedy(Graph g, count n);
      ~Greedy();      

      bool isValidEdge(const Edge& e);
      Edge getCandidateEdge();
      Edge bestEdge();
      void maintain(const Edge& inserted_edge, const std::vector<node>& affected_nodes);
    
      std::vector<Edge> doGreedy(count k);
      
      count node_rem(node i);

      Graph g_;
      index n_;
      node dummy_node;
      std::vector<index> core_;
      std::vector<core::GLIST::CoreComponent> nc_list_;
      std::vector<index> nc_ids_;
      
      // deg^{+}: node_[i].rem
      // order[i]: treap.Rank(i)
      core::GLIST glist_;
      std::vector<std::unordered_set<index>> bucket_1_;
      std::vector<std::unordered_set<index>> bucket_2_;

      // cache
      std::unordered_map<Edge, count> edge_gain_;
      std::unordered_set<Edge> added_edges_;
      std::unordered_set<Edge> evaluated_edges_;
      std::unordered_map<node, std::unordered_set<Edge>> n2e_dep_;

      // for inter core edge, store (u, dummy_node)
      std::unordered_map<Edge, count> edge_score_;

      // other
      count current_score_; //  \Delta^{*} f, the gain
      count core_max_; // maximum core value of all nodes
      count gain_max_; // maximum gain is bounded by the size of the largest CoreComponent
    };
  }
}

#endif
