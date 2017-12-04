#include "Greedy.h"

namespace NetworKit{
  namespace CoreMaximization {
    
    Greedy::Greedy(const Graph& g, count n): n_(n), core_(n), rem_(n), nc_list_(n), nc_ids_(n),  glist_(n), bucket_1_(n+1, std::unordered_set<index>()), bucket_2_(n+1, std::unordered_set<index>()){
      std::cerr << "BFS" << std::endl;
      glist_.CoreGuidedBFS(g, core_, nc_list_, nc_ids_);

      for(node i=0; i<n; i++){
	std::cerr << i << std::endl;
	if(core_[i] == node_rem(i)){
	  std::cerr << "inside: "<< i << std::endl;
	  core::GLIST::CoreComponent nc = nc_list_[nc_ids_[i]];
	  std::cerr << "bucket_1_.size()=" << bucket_1_.size() << std::endl;
	  std::cerr << "bucket_1_[" << nc.nodes.size() << "].insert " << i << std::endl;
	  bucket_1_[nc.nodes.size()].insert(i);
	  std::cerr << "nc.insert "<< i << std::endl;
	  nc.usable.insert(i);
	}
      }
    }
    
    // wrapper for glist::core::ListNode.rem
    count Greedy::node_rem(node i){
      return glist_.get_nodes()[i].rem;
    }
    
    Greedy::~Greedy() {}
    
  } // namespace NetworKit
}
