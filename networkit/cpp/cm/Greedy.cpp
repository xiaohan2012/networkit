#include "Greedy.h"

namespace NetworKit{
  namespace CoreMaximization {
    
    Greedy::Greedy(const Graph& g, count n): n_(n), dummy_node(n), core_(n), rem_(n), nc_list_(n), nc_ids_(n),  glist_(n), bucket_1_(n+1, std::unordered_set<index>()), bucket_2_(n+1, std::unordered_set<index>()){
      glist_.ComputeCore(g, true, core_);
      
      // std::cerr << "BFS" << std::endl;
      glist_.CoreGuidedBFS(g, core_, nc_list_, nc_ids_);
      // glist_.PrintNCList(nc_list_);

      // std::cerr << "bucket_1_.size()=" << bucket_1_.size() << std::endl;      
      for(node i=0; i<n; i++){
	// std::cerr << i << std::endl;
	if(core_[i] == node_rem(i)){
	  // std::cerr << "node: "<< i << std::endl;
	  // std::cerr << "nc id: "<< nc_ids_[i] << std::endl;
	  core::GLIST::CoreComponent& nc = nc_list_[nc_ids_[i]];	 
	  // std::cerr << "bucket_1_[" << nc.nodes.size() << "].insert " << i << std::endl;
	  bucket_1_[nc.nodes.size()].insert(i);
	  // std::cerr << "nc.insert "<< i << std::endl;
	  nc.usable.insert(i);
	  // for(auto e: nc.usable){
	  //   std::cerr << "e: " << e << std::endl;
	  // }
	}
      }

      for(index i=0; i<nc_list_.size(); i++){
	core::GLIST::CoreComponent& nc=nc_list_[i];
	bucket_2_[nc.nodes.size()].insert(i);
      }     
    }
    
    // wrapper for glist::core::ListNode.rem
    count Greedy::node_rem(node i){
      return glist_.get_nodes()[i].rem;
    }
    
    Greedy::~Greedy() {}
    
  } // namespace NetworKit
}
