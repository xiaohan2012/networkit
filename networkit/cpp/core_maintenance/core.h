#ifndef CORE_CORE_H_
#define CORE_CORE_H_

#include <vector>
#include "../Globals.h"
#include "../graph/Graph.h"

namespace NetworKit {
  namespace core {
    class CoreMaintenance {
    public:
      virtual ~CoreMaintenance() {}

      virtual void ComputeCore(const Graph& graph,
			       const bool init_idx, // initialize the index?
			       std::vector<node>& core) = 0;
      virtual void Insert(const node v1, const node v2,
			  Graph& graph,
			  std::vector<count>& core,
			  std::vector<node>& affected_nodes) = 0;
      virtual void Remove(const node v1, const node v2,
			  Graph& graph,
			  std::vector<count>& core) = 0;
      virtual void Check(const Graph& graph,
			 const std::vector<count>& core) const = 0;
    };
  }  // namespace core
}
#endif
