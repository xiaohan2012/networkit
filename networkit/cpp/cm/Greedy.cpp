#include "Greedy.h"

namespace NetworKit{
  namespace CoreMaximization {
    
    Greedy::Greedy(Graph g, count n): g_(g), n_(n), dummy_node(n), core_(n), nc_list_(n), nc_ids_(n),  glist_(n), bucket_1_(n+1, std::unordered_set<index>()), bucket_2_(n+1, std::unordered_set<index>()), current_score_(0){
      glist_.ComputeCore(g, true, core_);

      core_max_ = *std::max_element(core_.begin(), core_.end());
      
      // std::cerr << "BFS" << std::endl;
      glist_.CoreGuidedBFS(g, core_, nc_list_, nc_ids_);
      // glist_.PrintNCList(nc_list_);

      // gain_max_
      for(auto nc: nc_list_){
	if(nc.nodes.size() > gain_max_){
	  gain_max_ = nc.nodes.size();
	}
      }
	
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
      for(index i=current_score_ + 1; i < bucket_1_.size(); i++) {
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
      for(index i=current_score_+1; i < bucket_2_.size(); i++) {
	std::unordered_set<index> nc_ids = bucket_2_[i];
	for(index j: nc_ids){
	  // std::cerr << "nc_id=" << j << std::endl;
	  // within the CoreComponent
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
      throw std::runtime_error("no more candidate edges");
      // return Edge(dummy_node, dummy_node);
    }

    bool Greedy::isValidEdge(const Edge& e){
      return (e.u != dummy_node)  || (e.v != dummy_node);
    }
    
    Edge Greedy::bestEdge(){
      std::vector<node> affected_nodes;
      Edge best_e(dummy_node, dummy_node);
      while(current_score_ < gain_max_){
	std::cerr << current_score_ << " <= " << gain_max_ << std::endl;

	bool is_inter_core_edge = false;
	try{
	  // may throw exception
	  Edge e = getCandidateEdge();
	
	  // if inter-core edge
	  // randomly sample a higher core node as neighbor
	  // v is the dummy node
	  if(e.v == dummy_node){
	    is_inter_core_edge = true;
	    do{
	      e.v = g_.randomNode();
	      // (u, v) \not\in E and core(v) > core(u)
	    }
	    while(g_.hasEdge(e.u, e.v) || core_[e.v] <= core_[e.u]);
	  }
    
	  std::cerr << "trying (" << e.u << ", "<< e.v << ")" << std::endl;
	  glist_.FakeInsert(e.u, e.v, g_, core_, nc_ids_, affected_nodes);

	  // update cache
	  for(node u: affected_nodes){
	    n2e_dep_[u].insert(e);
	  }

	  count score = affected_nodes.size();
	  // cache the edge score
	  if(is_inter_core_edge){
	    Edge real_edge = Edge(e.u, dummy_node);
	    edge_score_[real_edge] = score;
	    evaluated_edges_.insert(real_edge);
	  }
	  else {
	    edge_score_[e] = score;
	    evaluated_edges_.insert(e);
	  }
	  
	  std::cerr << "affected nodes" << std::endl;
	  std::copy(affected_nodes.begin(), affected_nodes.end(), std::ostream_iterator<node>(std::cerr, " "));
	  affected_nodes.clear(); // don't forget the empty it
	
	  std::cerr << std::endl;
	
	  if(score > current_score_){
	    current_score_ = score;
	    best_e = e;
	    std::cerr << "best edge: (" << e.u << ", "<< e.v << ")" << std::endl; 
	    std::cerr << "current score: " << current_score_ << std::endl;
	  }
	}
	catch(std::runtime_error){
	  // no more edges
	  break;
	}
      }
      return best_e;
    }

    void Greedy::maintain(const Edge& e){
      std::copy(core_.begin(), core_.end(), std::ostream_iterator<node>(std::cerr, " "));

      // update nc_list_ and nc_ids_
      glist_.CoreGuidedBFS(g_, core_, nc_list_, nc_ids_);
	
      // adding e will change the incremental data structures
      std::vector<node> affected_nodes;
      int new_nc_id = glist_.FakeInsert(e.u, e.v, g_, core_, nc_ids_, affected_nodes);
      core::GLIST::CoreComponent& old_nc = nc_list_[nc_ids_[affected_nodes[0]]];
      core::GLIST::CoreComponent& new_nc = nc_list_[new_nc_id];
      for(node w: affected_nodes){
	// change core
	// change
      	new_nc.nodes.insert(w);
	if(core_[w] == node_rem(w)){ // core_ and rem_ should be updated
	  new_nc.usable.insert(w);
	}
	nc_ids_[w] = new_nc_id;

	old_nc.nodes.erase(w);
	old_nc.usable.erase(w); // check?
	
	for(Edge affected_e: n2e_dep_[w]){
	  edge_score_.erase(affected_e);
	  evaluated_edges_.erase(affected_e);	  
	}
      }

      // update gain_max_      
      if(new_nc.nodes.size() > gain_max_)
	gain_max_ = new_nc.nodes.size();

      // update current_score_
      current_score_ = 0;
      for(Edge e: evaluated_edges_){	
	if(edge_score_[e] > current_score_)
	  current_score_ = edge_score_[e];
      }      
    }
    
    std::vector<Edge> Greedy::doGreedy(count k){
      // select k edges greedily
      added_edges_.clear();
      std::vector<Edge> edges;
      for(index i=0; i<k; i++){
	Edge e = bestEdge();
	edges.push_back(e);
	added_edges_.insert(e);

	// insert the edge and update core_
	glist_.Insert(e.u, e.v, g_, core_);

	
	// update core_max_
	core_max_ = *std::max_element(core_.begin(), core_.end());

	// maintain the incremental data structures
	maintain(e);
      }
      return edges;
    }
  } // namespace NetworKit
}
