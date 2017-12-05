#ifndef CM_GREEDY_H_
#define CM_GREEDY_H_

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "../graph/Graph.h"
#include "../core_maintenance/glist.h"

namespace NetworKit{
  namespace CoreMaximization{
    class Greedy final {
    public:
      explicit Greedy(const Graph& g, count n);
      ~Greedy();      

      count node_rem(node i);
  
      index n_;
      node dummy_node;
      std::vector<index> core_;
      std::vector<index> rem_;
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
      std::unordered_map<node, Edge> n2e_dep_;

      // other
      count current_score; //  \Delta^{*} f
    };
  }
}

#endif
