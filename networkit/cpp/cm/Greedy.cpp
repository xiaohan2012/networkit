#include "Greedy.h"

namespace NetworKit{
  namespace CoreMaximization {
    
    Greedy::Greedy(Graph g, count n): g_(g), n_(n), dummy_node(n), core_(n), rem_(n), nc_list_(n), nc_ids_(n),  glist_(n), bucket_1_(n+1, std::unordered_set<index>()), bucket_2_(n+1, std::unordered_set<index>()), current_score_(0){
      glist_.ComputeCore(g, true, core_);

      core_max_ = *std::max_element(core_.begin(), core_.end());
      
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

    Edge Greedy::getCandidateEdge(){
      for(index i=current_score_ + 1; i < bucket_1_.size(); i++){
	// std::cerr << "i=" << i << std::endl;
	std::unordered_set<index>& B = bucket_1_[i];
	for(node u: B){
	  // std::cerr << "u=" << u << std::endl;
	  Edge cand_edge(u, dummy_node, true);
	  if(((g_.degree(u) < n_ - 1) && core_[u] < core_max_)
	     && (evaluated_edges_.find(cand_edge) ==  evaluated_edges_.end()) // not evaluated
	     && (added_edges_.find(cand_edge) ==  added_edges_.end()) // not added
	     ){
	    return cand_edge;
	  }
	}
      }
      for(index i=current_score_+1; i < bucket_2_.size(); i++){
	std::unordered_set<index> nc_ids = bucket_2_[i];
	for(index j: nc_ids){
	  // std::cerr << "nc_id=" << j << std::endl;
	  core::GLIST::CoreComponent &nc = nc_list_[j];
	  for(node u: nc.usable){
	    // std::cerr << "u=" << u << std::endl;
	    index u_rank = glist_.GetRank(u);
	    for(node v: nc.nodes){
	      // std::cerr << "v=" << v << std::endl;
	      if((!g_.hasEdge(u, v))
		 && (u != v)
		 && (u_rank < glist_.GetRank(v))){
		Edge cand_edge(u, v, true);
		if((evaluated_edges_.find(cand_edge) ==  evaluated_edges_.end())
		   && (added_edges_.find(cand_edge) ==  added_edges_.end())){
		  // std::cerr << "return " << u << ", " << v <<std::endl;
		  return cand_edge;
		}
	      }
	    }
	  }
	}
      }
      return Edge(dummy_node, dummy_node);
    }
  } // namespace NetworKit
}
