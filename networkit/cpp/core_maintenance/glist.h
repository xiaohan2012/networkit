// The @GLIST class implements the order-based core maintenance algorithm
// in the paper. There is another class called @LLIST which has slightly
// better performance but the class is not included here.
#ifndef CORE_GLIST_GLIST_H_
#define CORE_GLIST_GLIST_H_

#include <queue>
#include <iostream>

#include "core.h"

#include "gadget/heap.h"
#include "gadget/treap.h"

namespace NetworKit{
  namespace core {
    class GLIST final: public CoreMaintenance {
    public:
      explicit GLIST(const count n);
      ~GLIST();

      void ComputeCore(const Graph& graph,
		       const bool init_idx,
		       std::vector<count>& core);
      void Insert(const index v1, const index v2,
		  Graph& graph,
		  std::vector<count>& core,
		  std::vector<node>& affected_nodes);
      void Remove(const index v1, const index v2,
		  Graph& graph,
		  std::vector<count>& core);
      void Check(const Graph& graph,
		 const std::vector<count>& core) const;

      struct CoreComponent {
	std::unordered_set<index> nodes;
	std::unordered_set<index> usable;
      };
      
      void CoreGuidedBFS(const Graph& graph,
			 const std::vector<count>& core,
			 std::vector<CoreComponent>& nc_list,
			 std::vector<index>& nc_ids);

      void FakeInsert(const node v1, const node v2,
		     Graph& graph,
		     std::vector<count>& core,
		     const std::vector<node>& nc_ids,
		     std::vector<node>& affected_nodes);

      void Checkpoint();
      void Rollback();
      
      void PrintNCList(const std::vector<CoreComponent>& nc_list);
      /*
	for testing purpose only
	don't use unless it's not test
      */
      struct ListNode {
	count rem;
	count ext;
	index prev;
	index next;
      };
      
      std::vector<ListNode> get_nodes(){
	return this->node_;
      }

      index GetRank(const index v) {
	if (0 == rank_[v]) {
	  rank_[v] = tree_.Rank(v);
	  garbage_.push_back(v);
	}
	return rank_[v];
      }

            
    private:
      /* struct ListNode { */
      /* 	count rem; */
      /* 	count ext; */
      /* 	index prev; */
      /* 	index next; */
      /* }; */

      void Keep(const Graph& graph,
		const index v, const index K,
		const std::vector<count>& core,
		int& list_t, std::vector<int>& swap);
      void PropagateDismissal(const Graph& graph,
			      const index K, const index v,
			      std::vector<count>& core,
			      std::vector<int>& to_be_clear,
			      std::vector<int>& changed);

      count n_;
      std::vector<int> head_;
      std::vector<int> tail_;
      std::vector<ListNode> node_;
      std::vector<int> mcd_;
      std::vector<int> deg_;
      std::vector<int> rank_;
      std::vector<int> root_;
      std::vector<bool> evicted_;
      std::vector<bool> visited_;
      gadget::Treap tree_;
      gadget::MinHeap heap_;
      std::vector<int> garbage_;

      // for checkpoint
      // std::vector<int> cp_head_;
      // std::vector<int> cp_tail_;
      // std::vector<ListNode> cp_node_;
      // std::vector<int> cp_mcd_;
      // std::vector<int> cp_deg_;
      // std::vector<int> cp_rank_;
      // std::vector<int> cp_root_;
      // std::vector<bool> cp_evicted_;
      // std::vector<bool> cp_visited_;
      // gadget::Treap cp_tree_;
      // gadget::MinHeap cp_heap_;
      // std::vector<int> cp_garbage_;      
    };
  }  // namespace core
}
#endif
