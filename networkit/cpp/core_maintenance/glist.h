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
		       std::vector<node>& core);
      void Insert(const index v1, const index v2,
		  Graph& graph,
		  std::vector<index>& core);
      void Remove(const index v1, const index v2,
		  Graph& graph,
		  std::vector<index>& core);
      void Check(const Graph& graph,
		 const std::vector<index>& core) const;

      struct CoreComponent {
	std::unordered_set<index> nodes;
	std::unordered_set<index> usable;
      };
      
      void CoreGuidedBFS(const Graph& graph,
			 const std::vector<index>& core,
			 std::vector<CoreComponent>& nc_list,
			 std::vector<index>& nc_ids);

      int FakeInsert(const node v1, const node v2,
		     Graph& graph,
		     std::vector<count>& core,
		     const std::vector<node>& nc_ids,
		     std::vector<node>& affected_nodes);

      void FakeRemove(const index v1, const index v2,
			     Graph& graph,
			     std::vector<index>& core);
      
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
		const std::vector<index>& core,
		index& list_t, std::vector<index>& swap);
      void PropagateDismissal(const Graph& graph,
			      const index K, const index v,
			      std::vector<index>& core,
			      std::vector<index>& to_be_clear,
			      std::vector<index>& changed);

      index n_;
      std::vector<node> head_;
      std::vector<node> tail_;
      std::vector<ListNode> node_;
      std::vector<node> mcd_;
      std::vector<node> deg_;
      std::vector<node> rank_;
      std::vector<node> root_;
      std::vector<bool> evicted_;
      std::vector<bool> visited_;
      gadget::Treap tree_;
      gadget::MinHeap heap_;
      std::vector<node> garbage_;
    };
  }  // namespace core
}
#endif
