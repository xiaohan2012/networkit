#ifndef CM_GREEDY_H_
#define CM_GREEDY_H_

#include <vector>
#include <unordered_set>
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
      std::vector<index> core_;
      std::vector<index> rem_;
      std::vector<core::GLIST::CoreComponent> nc_list_;
      std::vector<index> nc_ids_;
      
      // deg^{+}: node_[i].rem
      // order[i]: treap.Rank(i)
      core::GLIST glist_;
      std::vector<std::unordered_set<index>> bucket_1_;
      std::vector<std::unordered_set<index>> bucket_2_;

    };
  }
}

#endif
