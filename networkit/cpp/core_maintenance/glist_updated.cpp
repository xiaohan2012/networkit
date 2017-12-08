#include "glist.h"

#include <algorithm>
#include <cstring>
#include <queue>
#include <set>
#include <map>

#include "defs.h"

#include "../Globals.h"
#include "../graph/Graph.h"

namespace NetworKit {
  namespace core {

    void GLIST::Checkpoint(){
      cp_head_     =     head_;   
      cp_tail_     =     tail_;   
      cp_node_     =     node_;   
      cp_mcd_      =     mcd_;    
      cp_deg_      =     deg_;    
      cp_rank_     =     rank_;   
      cp_root_     =     root_;   
      cp_evicted_  =     evicted_;
      cp_visited_  =     visited_;
      cp_tree_     =     tree_;   
      cp_heap_     =     heap_;   
      cp_garbage_  =     garbage_; 
    }
    
    void GLIST::Rollback(){
      head_     =     cp_head_;   
      tail_     =     cp_tail_;   
      node_     =     cp_node_;   
      mcd_      =     cp_mcd_;    
      deg_      =     cp_deg_;    
      rank_     =     cp_rank_;   
      root_     =     cp_root_;   
      evicted_  =     cp_evicted_;
      visited_  =     cp_visited_;
      tree_     =     cp_tree_;   
      heap_     =     cp_heap_;   
      garbage_  =     cp_garbage_; 
    }
    
    int GLIST::FakeInsert(const node v1, const node v2,
			  Graph& graph,
			  std::vector<count>& core,
			  const std::vector<node>& nc_ids,
			  std::vector<node>& affected_nodes) {
      /*
	updates the set of affected nodes 
	and return the new nc id
      */
      // prepare for restoration
      Checkpoint();
      
      // bound checking
      ASSERT(v1 >= 0);
      ASSERT(v2 >= 0);
      ASSERT(v1 < graph.numberOfNodes());
      ASSERT(v2 < graph.numberOfNodes());
      
      // insert the edge
      graph.addEdge(v1, v2);
      std::vector<count> cp_core = core; // is it copying?
	
      // update mcd
      if (core[v1] <= core[v2]) ++mcd_[v1];
      if (core[v2] <= core[v1]) ++mcd_[v2];
      // the source node and the current core number
      index src = v1, target = v2;
      const count K = core[v1] <= core[v2] ? core[v1] : core[v2];
      if ((core[v1] == core[v2] &&
	   tree_.Rank(v1) > tree_.Rank(v2)) ||
	  core[v1] > core[v2]) {
	src = v2;
	target = v1;
      }
      // update core number
      ++node_[src].rem;
      // there is no need to update the core numbers
      if (node_[src].rem <= K) {
	return -1;
      }
      // preparing the heap
      heap_.Insert(GetRank(src), src);
      //
      std::vector<index> swap;
      // the set of vertices, denoted as A, that doesn't need to be updated
      index list_h = -1, list_t = -1;
      for (index cur = head_[K]; n_ != cur; ) {
	if (heap_.Empty() || (node_[cur].ext == 0 && node_[cur].rem <= K)) {
	  const index start = cur;
	  const index end = heap_.Empty() ? tail_[K] : node_[heap_.Top().val].prev;
	  // advance the cur pointer
	  cur = node_[end].next;
	  // remove this sub-list and reinsert it into A
	  node_[node_[start].prev].next = node_[end].next;
	  node_[node_[end].next].prev = node_[start].prev;
	  node_[start].prev = n_;
	  node_[end].next = n_;
	  if (-1 == list_h) {
	    list_h = start;
	    list_t = end;
	  } else {
	    node_[start].prev = list_t;
	    node_[list_t].next = start;
	    list_t = end;
	  }
	  continue;
	}
	// update the heap
	// invariant: heap.Top().val == cur
	ASSERT(heap_.Top().val == cur);
	heap_.Delete(heap_.Top().key);
	// deal with cur
	const index next = node_[cur].next;
	const index cur_deg = node_[cur].ext + node_[cur].rem;
	if (likely(cur_deg <= K)) {
	  // insert into A
	  node_[node_[cur].prev].next = node_[cur].next;
	  node_[node_[cur].next].prev = node_[cur].prev;
	  if (likely(-1 != list_h)) {
	    node_[cur].next = n_;
	    node_[cur].prev = list_t;
	    node_[list_t].next = cur;
	    list_t = cur;
	  } else {
	    node_[cur].prev = node_[cur].next = n_;
	    list_h = list_t = cur;
	  }
	  node_[cur].rem = cur_deg;
	  node_[cur].ext = 0;
	  Keep(graph, cur, K, core, list_t, swap);
	} else {
	  // cur is temporarily marked as evicted, i.e.,
	  // its core number may be updated finally
	  evicted_[cur] = true;
	  for (const auto u : graph.neighbors(cur)) {
	    if (core[u] == core[cur] && GetRank(u) > rank_[cur]) {
	      ++node_[u].ext;
	      if (!heap_.Contains(rank_[u])) {
		heap_.Insert(rank_[u], u);
	      }
	    }
	  }
	}
	cur = next;
      }
      ASSERT(heap_.Empty());
      head_[K] = list_h;
      tail_[K] = list_t;

      // **DIFFERENCE** because of fake insert      
      // for (const index v : swap) {
      	// tree_.Delete(v, root_[K]);
      	// tree_.InsertAfter(v, node_[v].prev, root_[K]);
      // }
      
      // cope with those vertices whose core need to be updated
      if (evicted_[src]) {
	auto tail = -1; // tail
	for (auto v = src; n_ != v; v = node_[v].next) {
	  ++core[v];
	  
	  /* the difference */
	  // std::cerr << v <<  " is affected" << std::endl;
	  affected_nodes.push_back(v);
	  
	  node_[v].ext = 0;
	  tail = v;
	  // update mcd
	  for (const auto u : graph.neighbors(v)) {
	    if (evicted_[u]) continue;
	    if (K + 1 == core[u]) {
	      ++mcd_[u];
	    } else if (K == core[u]) {
	      --mcd_[v];
	    }
	  }
	  // remove from the current tree
	  // **DIFFERENCE** because of fake insert
	  // tree_.Delete(v, root_[K]);
	}
	for (auto v = tail; n_ != v; v = node_[v].prev) {
	  evicted_[v] = false;
	  // **DIFFERENCE** because of fake insert	  
	  // tree_.Insert(v, true, root_[K + 1]);
	}
	// merge list
	if (-1 == head_[K + 1]) {
	  head_[K + 1] = src;
	  tail_[K + 1] = tail;
	} else {
	  node_[head_[K + 1]].prev = tail;
	  node_[tail].next = head_[K + 1];
	  head_[K + 1] = src;
	}
      }
      for (const index v : garbage_) rank_[v] = 0;
      garbage_.clear();

      // std::cerr << "Remove(" << v1 << "," << v2 << ")" << std::endl; // 0, 4
      /* roll back */
      // FakeRemove(v1, v2, graph, core);
      Rollback();
      graph.removeEdge(v1, v2);
      core = cp_core;  // copy back
      return nc_ids[target];
      
    }

    void GLIST::CoreGuidedBFS(const Graph& graph,
			      const std::vector<index>& core,
			      std::vector<CoreComponent>& nc_list,
			      std::vector<index>& nc_ids){
      // std::cerr << "[          ] run " << std::endl;
      // reset the status
      std::fill(nc_ids.begin(), nc_ids.end(), 0);
      for(auto nc: nc_list){
	nc.nodes.clear();
	nc.usable.clear();
      }
      
      int n = graph.numberOfNodes();
      std::vector<bool> visited(n, false);
      std::queue<node> q;

      int nc_id = -1;
      for(index i: graph.nodes()){
	if(visited[i]) continue;

	nc_id++; //  a new component

	q.push(i);
	while(!q.empty()){	 
	  node u = q.front();
	  q.pop();

	  // std::cerr << "visiting: " << u << std::endl;
	  
	  visited[u] = true;
	  
	  nc_list[nc_id].nodes.insert(u);
	  nc_ids[u] = nc_id;
	  
	  for(node v: graph.neighbors(u)){
	    if(!visited[v] && core[u] == core[v]){
	      // std::cerr << "same core: " << v << std::endl;
	      visited[v] = true;
	      q.push(v);
	      // nc_list[nc_id].nodes.push_back(v);
	    }
	  }
	}
      }
    }

    void GLIST::PrintNCList(const std::vector<CoreComponent>& nc_list){
      for(index i=0; i < nc_list.size(); i++){
    	GLIST::CoreComponent nc = nc_list[i];
    	if(nc.nodes.size() > 0){
    	  std::cerr << "i=" << i << " with nodes: " << std::endl;
	  for(auto i: nc.nodes){
	    std::cerr << i << ", ";
	  }
    	}
	std::cerr << std::endl;
      }
    }    
  }
}
