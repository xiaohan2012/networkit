#include "glist.h"

#include <algorithm>
#include <cstring>
#include <queue>
#include <set>
#include <map>

// #include <google/dense_hash_set>

#include "defs.h"
#include "../Globals.h"
#include "../graph/Graph.h"

namespace NetworKit {
  namespace core {
    GLIST::GLIST(const count n): n_(n), tree_(n_), heap_((int)n_) {
      head_ = std::vector<int>(n_, -1);  // for O_k, store the head node of it
      tail_ = std::vector<int>(n_, -1); // for O_k, store the tail node of it
      node_ = std::vector<ListNode>(n_ + 1); // list of nodes
      mcd_  = std::vector<int>(n_, 0); // mcd value
      deg_  = std::vector<int>(n_, 0); // degree
      rank_ = std::vector<int>(n_, 0); // node rank
      root_ = std::vector<int>(n_, n_); // ?
      visited_ = std::vector<bool>(n_, false); //  for BFS
      evicted_ = std::vector<bool>(n_, false); //  eviction propagation

    }
    GLIST::~GLIST() {}

    void GLIST::ComputeCore(const Graph& graph,
			    const bool init_idx,
							std::vector<count>& core) {
      // compute the cores
      auto& deg = core;
      int max_deg = 0;
      for (index i = 0; i < n_; ++i) {
		// deg[i] = graph[i].size(); // stores the degree
		deg[i] = graph.degree(i);
		if (deg[i] > max_deg) {
		  max_deg = deg[i];  // update max_deg

		}

      }

      // bin[i] = number of nodes with degree i
      std::vector<int> bin(max_deg + 1, 0);
      for (index i = 0; i < n_; ++i) {
		++bin[deg[i]];

      }

      // bin[i] = number of nodes with degree <= i
      // bin[max_deg] = N
      // bin[0] = deg[0]
      // bin[1] = deg[0] + deg[1]
      // bin[2] = deg[0] + deg[1] + deg[2]
      int start = 0;
      for (int i = 0; i <= max_deg; ++i) {
		int temp = bin[i];
		bin[i] = start;
		start += temp;

      }

      // pos: node -> position
      // bin[i] <= pos[i] < bin[i+1]
      // vert: position -> node
      std::vector<int> vert(n_);
      std::vector<int> pos(n_);
      for (index i = 0; i < n_; ++i) {
		pos[i] = bin[deg[i]];
		vert[pos[i]] = i;
		++bin[deg[i]];   // increment bin[deg[i]] to reflects change of pos

      }

      // revert back bin[i] to the states before the previous loop
      for (int i = max_deg; i > 0; --i) {
		bin[i] = bin[i-1];

      }
      bin[0] = 0;

      int k = 0;
      auto vis = std::vector<bool>(n_, false);

      // for each position from 0..n-1
      for (index i = 0; i < n_; ++i) {
		const index v = vert[i]; // the node at position i
		if (deg[v] > k) k = deg[v]; // k
		ASSERT(bin[deg[v]] == i); // why?
		++bin[deg[v]]; // what does bin track?
		core[v] = k;
		vis[v] = true;
		int rem = 0;
		for (const int u : graph.neighbors(v)) { // u is v's neighbors
		  if (vis[u]) continue; // vis[u] true means u < v in order
		  ++rem; // inc remaining degree
		  const int pw = bin[deg[u]];
		  const int pu = pos[u];
		  if (pw != pu) { // exchange the position of w and u
			const int w = vert[pw];
			vert[pu] = w;
			pos[w] = pu;
			vert[pw] = u;
			pos[u] = pw;
		  }
		  ++bin[deg[u]];
		  --deg[u];  // degree(u) decrement
		  if (pos[u] == i + 1) {
			bin[deg[u]] = pos[u];

		  }
		}
		if (init_idx) { // build the linked list
		  node_[v].rem = rem;
		  if (head_[k] == -1) { // O_k
			node_[v].prev = node_[v].next = n_; //  to non-existent node n_
			head_[k] = tail_[k] = v;
		  } else {
			node_[v].next = n_;
			node_[v].prev = tail_[k]; // bidirectional linked list
			node_[tail_[k]].next = v;
			tail_[k] = v;

		  }
		  tree_.Insert(v, false, root_[k]); // ?
		}

      }
      if (init_idx) {
		for (index v = 0; v < n_; ++v) {
		  mcd_[v] = 0;
		  for (const int u : graph.neighbors(v)) {
			if (core[u] >= core[v]) {
			  ++mcd_[v];

			}

		  }

		}

      }

    }
    void GLIST::Insert(const node v1, const node v2,
		       Graph& graph,
		       std::vector<count>& core,
					   std::vector<node>& affected_nodes) {
      // insert the edge
      // graph[v1].push_back(v2);
      // graph[v2].push_back(v1);
	  // std::cerr << "insert edge" << std::endl;
      graph.addEdge(v1, v2);

      // update mcd
	  // std::cerr << "udpate mcd" << std::endl;
      if (core[v1] <= core[v2]) ++mcd_[v1];
      if (core[v2] <= core[v1]) ++mcd_[v2];
      // the source node and the current core number
      int src = v1;
      const int K = core[v1] <= core[v2] ? core[v1] : core[v2];
      if ((core[v1] == core[v2] &&
		   tree_.Rank(v1) > tree_.Rank(v2)) ||
		  core[v1] > core[v2]) {
		src = v2;

      }
	  // std::cerr << "update core number" << std::endl;
      // update core number
      ++node_[src].rem;
      // there is no need to update the core numbers
      if (node_[src].rem <= K) {
		return;

      }

	  // std::cerr << "heap.insert" << std::endl;
      // preparing the heap
      heap_.Insert(GetRank(src), src);
      //
      std::vector<int> swap;
      // the set of vertices, denoted as A, that doesn't need to be updated
	  // std::cerr << "for loop (1)" << std::endl;
      int list_h = -1, list_t = -1;
      for (int cur = head_[K]; n_ != cur; ) {
		if (heap_.Empty() || (node_[cur].ext == 0 && node_[cur].rem <= K)) {
		  const int start = cur;
		  const int end = heap_.Empty() ? tail_[K] : node_[heap_.Top().val].prev;
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
		const int next = node_[cur].next;
		const int cur_deg = node_[cur].ext + node_[cur].rem;
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

	  // std::cerr << "for loop (2)" << std::endl;
      for (const int v : swap) {
		tree_.Delete(v, root_[K]);
		tree_.InsertAfter(v, node_[v].prev, root_[K]);

      }

	  // std::cerr << "evivtion" << std::endl;
      // cope with those vertices whose core need to be updated
      if (evicted_[src]) {
		auto tail = -1; // tail
		for (auto v = src; n_ != v; v = node_[v].next) {
		  ++core[v];

		  // track which nodes are updated
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
		  tree_.Delete(v, root_[K]);

		}
		for (auto v = tail; n_ != v; v = node_[v].prev) {
		  evicted_[v] = false;
		  tree_.Insert(v, true, root_[K + 1]);

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

	  // std::cerr << "garbage" << std::endl;
      for (const int v : garbage_) rank_[v] = 0;
      garbage_.clear();

    }
    void GLIST::Remove(const node v1, const node v2,
					   Graph& graph,
					   std::vector<count>& core) {
      // remove the edge
      // graph[v1].erase(std::find(graph[v1].begin(), graph[v1].end(), v2));
      // graph[v2].erase(std::find(graph[v2].begin(), graph[v2].end(), v1));
	  // std::cout << "remove edge (" << v1 << ", " << v2 << ")" << std::endl;
      graph.removeEdge(v1, v2);

      // update the mcd values
      if (core[v1] <= core[v2]) --mcd_[v1];
      if (core[v2] <= core[v1]) --mcd_[v2];
      // set the root and core number
      const int root = core[v1] <= core[v2] ? v1 : v2;
      const int K = core[root];
      // update rem
      if (core[v1] == core[v2]) {
		if (tree_.Rank(v1) > tree_.Rank(v2)) {
		  --node_[v2].rem;

		} else {
		  --node_[v1].rem;

		}

      } else {
		--node_[root].rem;

      }
      // update cores
      std::vector<int> to_be_clear;
      std::vector<int> changed;
      if (core[v1] != core[v2]) {
		visited_[root] = true;
		deg_[root] = mcd_[root];
		to_be_clear.push_back(root);
		if (deg_[root] < K) {
		  PropagateDismissal(graph, K, root, core, to_be_clear, changed);

		}

      } else {
		visited_[v1] = true;
		deg_[v1] = mcd_[v1];
		to_be_clear.push_back(v1);
		if (deg_[v1] < K) {
		  PropagateDismissal(graph, K, v1, core, to_be_clear, changed);

		}
		if (!visited_[v2]) {
		  visited_[v2] = true;
		  deg_[v2] = mcd_[v2];
		  to_be_clear.push_back(v2);
		  if (deg_[v2] < K) {
			PropagateDismissal(graph, K, v2, core, to_be_clear, changed);

		  }

		}

      }
      // clear
      for (const int u : to_be_clear) {
		visited_[u] = false;
		deg_[u] = 0;

      }
      if (!changed.empty()) {
		while (n_ != head_[K] && evicted_[head_[K]]) {
		  head_[K] = node_[head_[K]].next;

		}
		while (n_ != tail_[K] && evicted_[tail_[K]]) {
		  tail_[K] = node_[tail_[K]].prev;

		}
		if (n_ == head_[K]) {
		  head_[K] = tail_[K] = -1;

		}
		for (const int v : changed) {
		  node_[v].rem = 0;
		  for (const int u : graph.neighbors(v)) {
			if (core[u] == K) {
			  --mcd_[u];
			  if (!evicted_[u] && GetRank(v) > GetRank(u)) {
				--node_[u].rem;

			  }

			} else if (core[u] == K - 1 && !evicted_[u]) {
			  ++mcd_[v];

			}
			if (core[u] >= K || (evicted_[u] && !visited_[u])) {
			  ++node_[v].rem;

			}

		  }
		  visited_[v] = true;

		}
		for (const auto v : changed) {
		  evicted_[v] = false;
		  visited_[v] = false;
		  tree_.Delete(v, root_[K]);
		  tree_.Insert(v, false, root_[K - 1]);
		  // remove from current list
		  node_[node_[v].next].prev = node_[v].prev;
		  node_[node_[v].prev].next = node_[v].next;
		  node_[v].next = node_[v].prev = n_;
		  // merge list
		  if (-1 == head_[K - 1]) {
			head_[K - 1] = tail_[K - 1] = v;

		  } else {
			node_[tail_[K - 1]].next = v;
			node_[v].prev = tail_[K - 1];
			tail_[K - 1] = v;

		  }

		}

      }
      for (const int g : garbage_) rank_[g] = 0;
      garbage_.clear();

    }
    void GLIST::Check(const Graph& graph,
					  const std::vector<count>& core) const {
      for (index v = 0; v < n_; ++v) {
		int local_mcd = 0;
		for (const auto u : graph.neighbors(v)) {
		  if (core[u] >= core[v]) ++local_mcd;

		}
		ASSERT(mcd_[v] == local_mcd);
		ASSERT(!visited_[v]);
		ASSERT(!evicted_[v]);
		ASSERT(rank_[v] == 0);
		ASSERT(deg_[v] == 0);

      }
      std::vector<bool> vis(n_, false);
      for (index v = 0; v < n_; ++v) {
		if (vis[v]) continue;
		const int K = core[v];
		int tail = -1;
		ASSERT(-1 != head_[K]);
		for (int tmp = head_[K]; n_ != tmp; tmp = node_[tmp].next) {
		  ASSERT(!vis[tmp]);
		  vis[tmp] = true;
		  tail = tmp;
		  ASSERT(core[tmp] == K);
		  ASSERT(node_[tmp].ext == 0);
		  if (n_ != node_[tmp].next) {
			ASSERT(node_[node_[tmp].next].prev == tmp);

		  }

		}
		ASSERT(tail_[K] == tail);
		ASSERT(node_[head_[K]].prev == n_);
		ASSERT(node_[tail_[K]].next == n_);

		for (int tmp = head_[K], rid = 0; n_ != tmp; tmp = node_[tmp].next) {
		  ASSERT(tree_.Rank(tmp) == ++rid);

		}
		for (int tmp = head_[K]; n_ != tmp; tmp = node_[tmp].next) {
		  int local = 0;
		  for (const auto u : graph.neighbors(tmp)) {
			if (core[u] > core[tmp] ||
				(core[u] == core[tmp] &&
				 tree_.Rank(u) > tree_.Rank(tmp))) {
			  ++local;

			}

		  }
		  ASSERT(local == node_[tmp].rem);
		  ASSERT(node_[tmp].rem <= K);

		}

      }
      ASSERT(garbage_.empty());
      ASSERT(heap_.Empty());

    }
    void GLIST::Keep(const Graph& graph,
		     const node v, const node K,
		     const std::vector<count>& core,
					 int& list_t, std::vector<int>& swap) {
      // update
      std::queue<int> bfs;
      for (const auto u : graph.neighbors(v)) {
		if (core[u] == core[v] && evicted_[u]) {
		  --node_[u].rem;
		  if (node_[u].rem + node_[u].ext <= K) {
			visited_[u] = true;
			bfs.push(u);

		  }

		}

      }
      while (!bfs.empty()) {
		const int u = bfs.front(); bfs.pop();
		visited_[u] = false;
		evicted_[u] = false;
		// insert u into the list
		node_[node_[u].prev].next = node_[u].next;
		node_[node_[u].next].prev = node_[u].prev;
		swap.push_back(u);
		node_[list_t].next = u;
		node_[u].next = n_;
		node_[u].prev = list_t;
		node_[u].rem += node_[u].ext;
		node_[u].ext = 0;
		// advance the tail of list
		list_t = u;
		// find more vertices to keep
		for (const auto w : graph.neighbors(u)) {
		  if (core[w] != core[u]) continue;
		  if (rank_[w] > rank_[v]) {
			--node_[w].ext;
			if (0 == node_[w].ext) {
			  heap_.Delete(rank_[w]);

			}

		  } else if (rank_[w] > rank_[u] && evicted_[w]) {
			--node_[w].ext;
			if (!visited_[w] && node_[w].ext + node_[w].rem <= K) {
			  visited_[w] = true;
			  bfs.push(w);

			}

		  } else if (evicted_[w]) {
			--node_[w].rem;
			if (!visited_[w] && node_[w].ext + node_[w].rem <= K) {
			  visited_[w] = true;
			  bfs.push(w);

			}

		  }

		}

      }

    }
    void GLIST::PropagateDismissal(const Graph& graph,
				   const node K, const node v,
				   std::vector<count>& core,
				   std::vector<int>& to_be_clear,
								   std::vector<int>& changed) {
      evicted_[v] = true;
      --core[v];
      changed.push_back(v);
      for (const auto u : graph.neighbors(v)) {
		if (K == core[u]) {
		  if (!visited_[u]) {
			deg_[u] = mcd_[u];
			visited_[u] = true;
			to_be_clear.push_back(u);
		  }
		  --deg_[u];
		  if (deg_[u] < K && !evicted_[u]) {
			PropagateDismissal(graph, K, u, core, to_be_clear, changed);
		  }

		}

      }

    }

  }  // namespace core
}

// #include "glist.h"

// #include <algorithm>
// #include <cstring>
// #include <queue>
// #include <set>
// #include <map>

// // #include <google/dense_hash_set>

// #include "defs.h"

// #include "../Globals.h"
// #include "../graph/Graph.h"

// namespace NetworKit {
//   namespace core {
//     GLIST::GLIST(const count n) {
//       n_ = n;
//       tree_ = gadget::Treap(n_);
//       heap_ = gadget::MinHeap(n_);
//       head_ = std::vector<int>(n_, -1);
//       tail_ = std::vector<int>(n_, -1);
//       node_ = std::vector<ListNode>(n_ + 1);
//       mcd_  = std::vector<int>(n_, 0);
//       deg_  = std::vector<int>(n_, 0);
//       rank_ = std::vector<int>(n_, 0);
//       root_ = std::vector<int>(n_, n_);
//       visited_ = std::vector<bool>(n_, false);
//       evicted_ = std::vector<bool>(n_, false);

//       Checkpoint();
//     }
//     GLIST::~GLIST() {}

//     void GLIST::ComputeCore(const Graph& graph,
// 			    const bool init_idx,
// 			    std::vector<node>& core) {
//       // compute the cores
//       auto& deg = core;
//       count max_deg = 0;
//       for (index i = 0; i < n_; ++i) {
// 	// deg[i] = graph[i].size();
// 	deg[i] = graph.degree(i);
// 	if (deg[i] > max_deg) {
// 	  max_deg = deg[i];
// 	}
//       }
//       std::vector<count> bin(max_deg + 1, 0);
//       for (index i = 0; i < n_; ++i) {
// 	++bin[deg[i]];
//       }
//       index start = 0;
//       for (index i = 0; i <= max_deg; ++i) {
// 	count temp = bin[i];
// 	bin[i] = start;
// 	start += temp;
//       }
//       std::vector<node> vert(n_);
//       std::vector<index> pos(n_);
//       for (index i = 0; i < n_; ++i) {
// 	pos[i] = bin[deg[i]];
// 	vert[pos[i]] = i;
// 	++bin[deg[i]];
//       }
//       for (index i = max_deg; i > 0; --i) {
// 	bin[i] = bin[i-1];
//       }
//       bin[0] = 0;
//       index k = 0;
//       auto vis = std::vector<bool>(n_, false);
//       for (index i = 0; i < n_; ++i) {
// 	const node v = vert[i];
// 	if (deg[v] > k) k = deg[v];
// 	ASSERT(bin[deg[v]] == i);
// 	++bin[deg[v]];
// 	core[v] = k;
// 	vis[v] = true;
// 	count rem = 0;
// 	for (const node u : graph.neighbors(v)) {
// 	  if (vis[u]) continue;
// 	  ++rem;
// 	  const index pw = bin[deg[u]];
// 	  const index pu = pos[u];
// 	  if (pw != pu) {
// 	    const node w = vert[pw];
// 	    vert[pu] = w;
// 	    pos[w] = pu;
// 	    vert[pw] = u;
// 	    pos[u] = pw;
// 	  }
// 	  ++bin[deg[u]];
// 	  --deg[u];
// 	  if (pos[u] == i + 1) {
// 	    bin[deg[u]] = pos[u];
// 	  }
// 	}
// 	if (init_idx) {
// 	  node_[v].rem = rem;
// 	  if (head_[k] == -1) {
// 	    node_[v].prev = node_[v].next = n_;
// 	    head_[k] = tail_[k] = v;
// 	  } else {
// 	    node_[v].next = n_;
// 	    node_[v].prev = tail_[k];
// 	    node_[tail_[k]].next = v;
// 	    tail_[k] = v;
// 	  }
// 	  tree_.Insert(v, false, root_[k]);
// 	}
//       }
//       if (init_idx) {
// 	for (index v = 0; v < n_; ++v) {
// 	  mcd_[v] = 0;
// 	  for (const node u : graph.neighbors(v)) {
// 	    if (core[u] >= core[v]) {
// 	      ++mcd_[v];
// 	    }
// 	  }
// 	}
//       }
//     }
//     void GLIST::Insert(const node v1, const node v2,
// 		       Graph& graph,
// 		       std::vector<count>& core) {
//       // insert the edge
//       // graph[v1].push_back(v2);
//       // graph[v2].push_back(v1);
//       graph.addEdge(v1, v2);
//       // update mcd
//       if (core[v1] <= core[v2]) ++mcd_[v1];
//       if (core[v2] <= core[v1]) ++mcd_[v2];
//       // the source node and the current core number
//       index src = v1;
//       const count K = core[v1] <= core[v2] ? core[v1] : core[v2];
//       if ((core[v1] == core[v2] &&
// 	   tree_.Rank(v1) > tree_.Rank(v2)) ||
// 	  core[v1] > core[v2]) {
// 	src = v2;
//       }
//       // update core number
//       ++node_[src].rem;
//       // there is no need to update the core numbers
//       if (node_[src].rem <= K) {
// 	return;
//       }
//       // preparing the heap
//       heap_.Insert(GetRank(src), src);
//       //
//       std::vector<index> swap;
//       // the set of vertices, denoted as A, that doesn't need to be updated
//       index list_h = -1, list_t = -1;
//       for (index cur = head_[K]; n_ != cur; ) {
// 	if (heap_.Empty() || (node_[cur].ext == 0 && node_[cur].rem <= K)) {
// 	  const index start = cur;
// 	  const index end = heap_.Empty() ? tail_[K] : node_[heap_.Top().val].prev;
// 	  // advance the cur pointer
// 	  cur = node_[end].next;
// 	  // remove this sub-list and reinsert it into A
// 	  node_[node_[start].prev].next = node_[end].next;
// 	  node_[node_[end].next].prev = node_[start].prev;
// 	  node_[start].prev = n_;
// 	  node_[end].next = n_;
// 	  if (-1 == list_h) {
// 	    list_h = start;
// 	    list_t = end;
// 	  } else {
// 	    node_[start].prev = list_t;
// 	    node_[list_t].next = start;
// 	    list_t = end;
// 	  }
// 	  continue;
// 	}
// 	// update the heap
// 	// invariant: heap.Top().val == cur
// 	ASSERT(heap_.Top().val == cur);
// 	heap_.Delete(heap_.Top().key);
// 	// deal with cur
// 	const index next = node_[cur].next;
// 	const index cur_deg = node_[cur].ext + node_[cur].rem;
// 	if (likely(cur_deg <= K)) {
// 	  // insert into A
// 	  node_[node_[cur].prev].next = node_[cur].next;
// 	  node_[node_[cur].next].prev = node_[cur].prev;
// 	  if (likely(-1 != list_h)) {
// 	    node_[cur].next = n_;
// 	    node_[cur].prev = list_t;
// 	    node_[list_t].next = cur;
// 	    list_t = cur;
// 	  } else {
// 	    node_[cur].prev = node_[cur].next = n_;
// 	    list_h = list_t = cur;
// 	  }
// 	  node_[cur].rem = cur_deg;
// 	  node_[cur].ext = 0;
// 	  Keep(graph, cur, K, core, list_t, swap);
// 	} else {
// 	  // cur is temporarily marked as evicted, i.e.,
// 	  // its core number may be updated finally
// 	  evicted_[cur] = true;
// 	  for (const auto u : graph.neighbors(cur)) {
// 	    if (core[u] == core[cur] && GetRank(u) > rank_[cur]) {
// 	      ++node_[u].ext;
// 	      if (!heap_.Contains(rank_[u])) {
// 		heap_.Insert(rank_[u], u);
// 	      }
// 	    }
// 	  }
// 	}
// 	cur = next;
//       }
//       ASSERT(heap_.Empty());
//       head_[K] = list_h;
//       tail_[K] = list_t;
//       for (const index v : swap) {
// 	tree_.Delete(v, root_[K]);
// 	tree_.InsertAfter(v, node_[v].prev, root_[K]);
//       }
//       // cope with those vertices whose core need to be updated
//       if (evicted_[src]) {
// 	auto tail = -1; // tail
// 	for (auto v = src; n_ != v; v = node_[v].next) {
// 	  ++core[v];
// 	  node_[v].ext = 0;
// 	  tail = v;
// 	  // update mcd
// 	  for (const auto u : graph.neighbors(v)) {
// 	    if (evicted_[u]) continue;
// 	    if (K + 1 == core[u]) {
// 	      ++mcd_[u];
// 	    } else if (K == core[u]) {
// 	      --mcd_[v];
// 	    }
// 	  }
// 	  // remove from the current tree
// 	  tree_.Delete(v, root_[K]);
// 	}
// 	for (auto v = tail; n_ != v; v = node_[v].prev) {
// 	  evicted_[v] = false;
// 	  tree_.Insert(v, true, root_[K + 1]);
// 	}
// 	// merge list
// 	if (-1 == head_[K + 1]) {
// 	  head_[K + 1] = src;
// 	  tail_[K + 1] = tail;
// 	} else {
// 	  node_[head_[K + 1]].prev = tail;
// 	  node_[tail].next = head_[K + 1];
// 	  head_[K + 1] = src;
// 	}
//       }
//       for (const index v : garbage_) rank_[v] = 0;
//       garbage_.clear();
//     }
//     void GLIST::Remove(const index v1, const index v2,
// 		       Graph& graph,
// 		       std::vector<index>& core) {
//       // remove the edge
//       // graph[v1].erase(std::find(graph[v1].begin(), graph[v1].end(), v2));
//       // graph[v2].erase(std::find(graph[v2].begin(), graph[v2].end(), v1));
//       graph.removeEdge(v1, v2);
//       // update the mcd values
//       if (core[v1] <= core[v2]) --mcd_[v1];
//       if (core[v2] <= core[v1]) --mcd_[v2];
//       // set the root and core number
//       const index root = core[v1] <= core[v2] ? v1 : v2;
//       const index K = core[root];
//       // update rem
//       if (core[v1] == core[v2]) {
// 	if (tree_.Rank(v1) > tree_.Rank(v2)) {
// 	  --node_[v2].rem;
// 	} else {
// 	  --node_[v1].rem;
// 	}
//       } else {
// 	--node_[root].rem;
//       }
//       // update cores
//       std::vector<index> to_be_clear;
//       std::vector<index> changed;
//       if (core[v1] != core[v2]) {
// 	visited_[root] = true;
// 	deg_[root] = mcd_[root];
// 	to_be_clear.push_back(root);
// 	if (deg_[root] < K) {
// 	  PropagateDismissal(graph, K, root, core, to_be_clear, changed);
// 	}
//       } else {
// 	visited_[v1] = true;
// 	deg_[v1] = mcd_[v1];
// 	to_be_clear.push_back(v1);
// 	if (deg_[v1] < K) {
// 	  PropagateDismissal(graph, K, v1, core, to_be_clear, changed);
// 	}
// 	if (!visited_[v2]) {
// 	  visited_[v2] = true;
// 	  deg_[v2] = mcd_[v2];
// 	  to_be_clear.push_back(v2);
// 	  if (deg_[v2] < K) {
// 	    PropagateDismissal(graph, K, v2, core, to_be_clear, changed);
// 	  }
// 	}
//       }
//       // clear
//       for (const index u : to_be_clear) {
// 	visited_[u] = false;
// 	deg_[u] = 0;
//       }
//       if (!changed.empty()) {
// 	while (n_ != head_[K] && evicted_[head_[K]]) {
// 	  head_[K] = node_[head_[K]].next;
// 	}
// 	while (n_ != tail_[K] && evicted_[tail_[K]]) {
// 	  tail_[K] = node_[tail_[K]].prev;
// 	}
// 	if (n_ == head_[K]) {
// 	  head_[K] = tail_[K] = -1;
// 	}
// 	for (const index v : changed) {
// 	  node_[v].rem = 0;
// 	  for (const index u : graph.neighbors(v)) {
// 	    if (core[u] == K) {
// 	      --mcd_[u];
// 	      if (!evicted_[u] && GetRank(v) > GetRank(u)) {
// 		--node_[u].rem;
// 	      }
// 	    } else if (core[u] == K - 1 && !evicted_[u]) {
// 	      ++mcd_[v];
// 	    }
// 	    if (core[u] >= K || (evicted_[u] && !visited_[u])) {
// 	      ++node_[v].rem;
// 	    }
// 	  }
// 	  visited_[v] = true;
// 	}
// 	for (const auto v : changed) {
// 	  evicted_[v] = false;
// 	  visited_[v] = false;
// 	  tree_.Delete(v, root_[K]);
// 	  tree_.Insert(v, false, root_[K - 1]);
// 	  // remove from current list
// 	  node_[node_[v].next].prev = node_[v].prev;
// 	  node_[node_[v].prev].next = node_[v].next;
// 	  node_[v].next = node_[v].prev = n_;
// 	  // merge list
// 	  if (-1 == head_[K - 1]) {
// 	    head_[K - 1] = tail_[K - 1] = v;
// 	  } else {
// 	    node_[tail_[K - 1]].next = v;
// 	    node_[v].prev = tail_[K - 1];
// 	    tail_[K - 1] = v;
// 	  }
// 	}
//       }
//       for (const index g : garbage_) rank_[g] = 0;
//       garbage_.clear();
//     }
//     void GLIST::Check(const Graph& graph,
// 		      const std::vector<index>& core) const {
//       for (index v = 0; v < n_; ++v) {
// 	index local_mcd = 0;
// 	for (const auto u : graph.neighbors(v)) {
// 	  if (core[u] >= core[v]) ++local_mcd;
// 	}
// 	ASSERT(mcd_[v] == local_mcd);
// 	ASSERT(!visited_[v]);
// 	ASSERT(!evicted_[v]);
// 	ASSERT(rank_[v] == 0);
// 	ASSERT(deg_[v] == 0);
//       }
//       std::vector<bool> vis(n_, false);
//       for (index v = 0; v < n_; ++v) {
// 	if (vis[v]) continue;
// 	const index K = core[v];
// 	index tail = -1;
// 	ASSERT(-1 != head_[K]);
// 	for (index tmp = head_[K]; n_ != tmp; tmp = node_[tmp].next) {
// 	  ASSERT(!vis[tmp]);
// 	  vis[tmp] = true;
// 	  tail = tmp;
// 	  ASSERT(core[tmp] == K);
// 	  ASSERT(node_[tmp].ext == 0);
// 	  if (n_ != node_[tmp].next) {
// 	    ASSERT(node_[node_[tmp].next].prev == tmp);
// 	  }
// 	}
// 	ASSERT(tail_[K] == tail);
// 	ASSERT(node_[head_[K]].prev == n_);
// 	ASSERT(node_[tail_[K]].next == n_);

// 	for (index tmp = head_[K], rid = 0; n_ != tmp; tmp = node_[tmp].next) {
// 	  ASSERT(tree_.Rank(tmp) == ++rid);
// 	}
// 	for (index tmp = head_[K]; n_ != tmp; tmp = node_[tmp].next) {
// 	  index local = 0;
// 	  for (const auto u : graph.neighbors(tmp)) {
// 	    if (core[u] > core[tmp] ||
// 		(core[u] == core[tmp] &&
// 		 tree_.Rank(u) > tree_.Rank(tmp))) {
// 	      ++local;
// 	    }
// 	  }
// 	  ASSERT(local == node_[tmp].rem);
// 	  ASSERT(node_[tmp].rem <= K);
// 	}
//       }
//       ASSERT(garbage_.empty());
//       ASSERT(heap_.Empty());
//     }
//     void GLIST::Keep(const Graph& graph,
// 		     const index v, const index K,
// 		     const std::vector<index>& core,
// 		     index& list_t, std::vector<index>& swap) {
//       // update
//       std::queue<index> bfs;
//       for (const auto u : graph.neighbors(v)) {
// 	if (core[u] == core[v] && evicted_[u]) {
// 	  --node_[u].rem;
// 	  if (node_[u].rem + node_[u].ext <= K) {
// 	    visited_[u] = true;
// 	    bfs.push(u);
// 	  }
// 	}
//       }
//       while (!bfs.empty()) {
// 	const index u = bfs.front(); bfs.pop();
// 	visited_[u] = false;
// 	evicted_[u] = false;
// 	// insert u into the list
// 	node_[node_[u].prev].next = node_[u].next;
// 	node_[node_[u].next].prev = node_[u].prev;
// 	swap.push_back(u);
// 	node_[list_t].next = u;
// 	node_[u].next = n_;
// 	node_[u].prev = list_t;
// 	node_[u].rem += node_[u].ext;
// 	node_[u].ext = 0;
// 	// advance the tail of list
// 	list_t = u;
// 	// find more vertices to keep
// 	for (const auto w : graph.neighbors(u)) {
// 	  if (core[w] != core[u]) continue;
// 	  if (rank_[w] > rank_[v]) {
// 	    --node_[w].ext;
// 	    if (0 == node_[w].ext) {
// 	      heap_.Delete(rank_[w]);
// 	    }
// 	  } else if (rank_[w] > rank_[u] && evicted_[w]) {
// 	    --node_[w].ext;
// 	    if (!visited_[w] && node_[w].ext + node_[w].rem <= K) {
// 	      visited_[w] = true;
// 	      bfs.push(w);
// 	    }
// 	  } else if (evicted_[w]) {
// 	    --node_[w].rem;
// 	    if (!visited_[w] && node_[w].ext + node_[w].rem <= K) {
// 	      visited_[w] = true;
// 	      bfs.push(w);
// 	    }
// 	  }
// 	}
//       }
//     }
//     void GLIST::PropagateDismissal(const Graph& graph,
// 				   const index K, const index v,
// 				   std::vector<index>& core,
// 				   std::vector<index>& to_be_clear,
// 				   std::vector<index>& changed) {
//       evicted_[v] = true;
//       --core[v];
//       changed.push_back(v);
//       for (const auto u : graph.neighbors(v)) {
// 	if (K == core[u]) {
// 	  if (!visited_[u]) {
// 	    deg_[u] = mcd_[u];
// 	    visited_[u] = true;
// 	    to_be_clear.push_back(u);
// 	  }
// 	  --deg_[u];
// 	  if (deg_[u] < K && !evicted_[u]) {
// 	    PropagateDismissal(graph, K, u, core, to_be_clear, changed);
// 	  }
// 	}
//       }
//     }
//   }  // namespace core
// }
