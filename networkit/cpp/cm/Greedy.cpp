#include "Greedy.h"

namespace NetworKit{
  namespace CoreMaximization {
    
    Greedy::Greedy(Graph g, count n): g_(g), n_(n), dummy_node(n), core_(n), nc_list_(n), nc_ids_(n),  glist_(n), bucket_1_(n+1, std::unordered_set<index>()), bucket_2_(n+1, std::unordered_set<index>()), current_score_(0), best_edge_(dummy_node, dummy_node) {
      glist_.ComputeCore(g, true, core_);

      core_max_ = *std::max_element(core_.begin(), core_.end());
      
      // std::cerr << "BFS" << std::endl;
      glist_.CoreGuidedBFS(g, core_, nc_list_, nc_ids_);
      // glist_.PrintNCList(nc_list_);

      // gain_max_
      // clique shouldn't be counted here
      gain_max_ = 0;
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
	if(nc.nodes.size() > 1)
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
	  if(((g_.degree(u) < n_ - 1) && core_[u] < core_max_) // 2nd condition, needs to link to higher core
	     && (proposed_edges_.find(cand_edge) ==  proposed_edges_.end()) // not proposed
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
		if((proposed_edges_.find(cand_edge) ==  proposed_edges_.end()) // not proposed
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

    bool Greedy::isIntercoreEdge(const Edge& e){
      return e.v == dummy_node;
    }    
    
    void Greedy::processIntercoreEdge(Edge& e){
      do{
	e.v = g_.randomNode();
	      // (u, v) \not\in E and core(v) > core(u)
      }
      while(g_.hasEdge(e.u, e.v) || core_[e.v] <= core_[e.u]);
    }
    
    Edge Greedy::bestEdge(){
      proposed_edges_.clear();
      
      std::vector<node> affected_nodes;
      // Edge best_e(dummy_node, dummy_node);
      while(current_score_ < gain_max_){
	std::cerr << "current score: " << current_score_ << " <= " << "gain_max_: " << gain_max_ << std::endl;

	bool is_inter_core_edge = false;
	try{
	  // may throw exception
	  Edge e = getCandidateEdge();
	  proposed_edges_.insert(e);
	  // if inter-core edge
	  // randomly sample a higher core node as neighbor
	  // v is the dummy node
	  if(isIntercoreEdge(e)){
	    is_inter_core_edge = true;
	    processIntercoreEdge(e);
	  }
    
	  std::cerr << "trying (" << e.u << ", "<< e.v << ")" << std::endl;
	  glist_.FakeInsert(e.u, e.v, g_, core_, nc_ids_, affected_nodes);

	  count score;
	  // the edge is not evaluated yet
	  // compute the score and update the cache
	  if((evaluated_edges_.find(e) ==  evaluated_edges_.end())) {
	    // update cache
	    for(node u: affected_nodes){
	      n2e_dep_[u].insert(e);
	    }

	    score = affected_nodes.size();
	    // cache the edge score
	    if(is_inter_core_edge){
	      Edge real_edge(e.u, dummy_node);
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
	  }
	  else{ // it's evaluated
	    if (is_inter_core_edge)
	      score = edge_score_[Edge(e.u, dummy_node)];
	    else 
	      score = edge_score_[e];
	  }
	
	  if(score > current_score_) {
	    current_score_ = score;
	    best_edge_ = e;
	    std::cerr << "best edge: (" << e.u << ", "<< e.v << ")" << std::endl; 
	    std::cerr << "current score: " << current_score_ << std::endl;
	  }
	}
	catch(std::runtime_error){
	  // no more edges
	  break;
	}
      }
      return best_edge_;
    }

    void Greedy::maintain(const Edge& inserted_edge, const std::vector<node>& affected_nodes){
      core_max_ = *std::max_element(core_.begin(), core_.end());
      
      std::cerr << "core number" << std::endl;
      std::copy(core_.begin(), core_.end(),
		std::ostream_iterator<node>(std::cerr, " "));
      std::cerr << std::endl;

      std::cerr << "affected nodes:" << std::endl;
      std::copy(affected_nodes.begin(), affected_nodes.end(),
		std::ostream_iterator<node>(std::cerr, " "));
      std::cerr << std::endl;
	
      node n = affected_nodes.front();
      index nc_id_prev = nc_ids_[n];
      std::cerr << "prev nv id: " << nc_id_prev << std::endl;
      core::GLIST::CoreComponent& nc_prev = nc_list_[nc_id_prev];
      int size_prev = nc_prev.nodes.size();

      std::cerr << "bucket erase" << std::endl;
      std::cerr << "nc_prev.usable.size() = " << nc_prev.usable.size() << std::endl;

      // std::cerr << "before:" << std::endl;      
      // std::copy(bucket_1_[size_prev].begin(),
      // 		bucket_1_[size_prev].end(),
      // 		std::ostream_iterator<node>(std::cerr, " "));      
      
      for(auto n: nc_prev.usable)
	bucket_1_[size_prev].erase(n);

      // std::cerr << "after::" << std::endl;            
      // std::copy(bucket_1_[size_prev].begin(),
      // 		bucket_1_[size_prev].end(),
      // 		std::ostream_iterator<node>(std::cerr, " "));
      bucket_2_[size_prev].erase(nc_id_prev);

      std::cerr << "getting affected nc ids..." << std::endl;
      std::unordered_set<index> affected_nc_ids;
      for(node x: affected_nodes) {
	for(node y: g_.neighbors(x)){
	  if((core_[x] == core_[y]) && (nc_ids_[y] != nc_id_prev))
	    affected_nc_ids.insert(nc_ids_[y]);
	}
      }
      std::cerr << "affected nc ids:" << std::endl;
      std::copy(affected_nc_ids.begin(), affected_nc_ids.end(),
		std::ostream_iterator<node>(std::cerr, " "));
      std::cerr << std::endl;      

      // remove the nodes and nc ids from bucket_1_ and bucket_2_
      std::cerr << "remove the nodes and nc ids from bucket_1_ and bucket_2_" << std::endl;      
      for(int nc_id: affected_nc_ids){
	core::GLIST::CoreComponent& nc = nc_list_[nc_id];
	int size = nc.nodes.size();
	for(auto n: nc.usable) bucket_1_[size].erase(n);
	// bucket_1_[size].erase(nc.usable.begin(), nc.usable.end());
	bucket_2_[size].erase(nc_id);
      }
      std::cerr << "finding the anchor nc id_" << std::endl;      
      int anchor_nc_id = 0;
      if(affected_nc_ids.size() > 0){
	anchor_nc_id = (* affected_nc_ids.begin());
      }
      else{
	// search until found an empty slot to save nc_id
	while(nc_list_[anchor_nc_id].nodes.size() != 0)
	  anchor_nc_id++;
      }
      std::cerr << "anchor nc id: " << anchor_nc_id << std::endl;

      // remove anchor_nc_id from affected_nc_ids
      std::cerr << "adding nodes to anchor nc" << std::endl;
      core::GLIST::CoreComponent& anchor_nc = nc_list_[anchor_nc_id];
      for(int nc_id: affected_nc_ids){
	core::GLIST::CoreComponent& this_nc = nc_list_[nc_id];
	anchor_nc.nodes.insert(this_nc.nodes.begin(), this_nc.nodes.end());
	anchor_nc.usable.insert(this_nc.usable.begin(), this_nc.usable.end());
	for(node x: this_nc.nodes)
	  nc_ids_[x] = anchor_nc_id;
	this_nc.nodes.clear();
	this_nc.usable.clear();
      }

      std::cerr << "removing affeced nodes from nc_prev" << std::endl;
      std::unordered_set<node> affected_node_set(affected_nodes.begin(), affected_nodes.end());
      for(auto n: affected_node_set) nc_prev.nodes.erase(n);
      for(auto n: affected_node_set) nc_prev.usable.erase(n);


      anchor_nc.nodes.insert(affected_node_set.begin(), affected_node_set.end());
      for(node z: affected_nodes){
	nc_ids_[z] = anchor_nc_id;
	if(core_[z] == node_rem(z))
	  anchor_nc.usable.insert(z);
	
	for(Edge affected_e: n2e_dep_[z]){
	  edge_score_.erase(affected_e);
	  evaluated_edges_.erase(affected_e);	  
	}
	n2e_dep_[z].erase(inserted_edge);
      }

      // add nodes in anchor_nc to bucket_1_ and bucket_2_
      // should skip nodes in max core (which happens to be clique)
      for(node n: anchor_nc.usable)
	if(core_[n] < core_max_) // only nodes in lower core 
	  bucket_1_[anchor_nc.nodes.size()].insert(n);

      bucket_2_[anchor_nc.nodes.size()].insert(anchor_nc_id);
      
      
      // udpate the remaining of nc_prev
      size_prev = nc_prev.nodes.size();
      if(nc_prev.usable.size() > 0){
	bucket_1_[size_prev].insert(nc_prev.usable.begin(), nc_prev.usable.end());
	if(nc_prev.nodes.size()>1)
	  bucket_2_[size_prev].insert(nc_id_prev);

      }

      gain_max_ = 0;
      for(auto nc: nc_list_){
	if(nc.nodes.size() > gain_max_){
	  gain_max_ = nc.nodes.size();
	}
      }
      
      // update current_score_
      current_score_ = 0;
      for(Edge e: evaluated_edges_){	
	if(edge_score_[e] > current_score_){
	  best_edge_  = e;
	  current_score_ = edge_score_[e];
	}
      }
    }
    
    std::vector<Edge> Greedy::doGreedy(count k){
      // select k edges greedily
      added_edges_.clear();
      std::vector<Edge> edges;
      std::vector<node> affected_nodes;
      for(index i=0; i<k; i++){
	Edge e = bestEdge();
	std::cerr << "SELECTED EDGE: (" << e.u << ", " << e.v << ")" << std::endl;

	if(isIntercoreEdge(e))
	  processIntercoreEdge(e);
	
	edges.push_back(e);
	added_edges_.insert(e);

	// insert the edge and update core_
	glist_.Insert(e.u, e.v, g_, core_, affected_nodes);

	std::cerr << "core number" << std::endl;
	std::copy(core_.begin(), core_.end(),
		  std::ostream_iterator<node>(std::cerr, " "));
	std::cerr << "affected nodes:" << std::endl;
	std::copy(affected_nodes.begin(), affected_nodes.end(),
		  std::ostream_iterator<node>(std::cerr, " "));		

	// maintain the incremental data structures
	maintain(e, affected_nodes);

	affected_nodes.clear();
      }
      return edges;
    }
  } // namespace NetworKit
}
