#ifndef CORE_GADGET_GADGET_H_
#define CORE_GADGET_GADGET_H_

#include <utility>
#include <vector>
#include "../../Globals.h"
#include "../../graph/Graph.h"

namespace NetworKit{
  namespace gadget {
    void RepeatWith(const char symbol, const int repeat);
    Graph ReadGraph(const char* const path,
		    count* const n, count* const m);
    Graph ReadTempEdgesS(const char* const path,
			 count* const n, count* const m);
    Graph ReadEdgesS(const char* const path,
		     count* const n, count* const m);
  }  // namespace gadget
}
#endif
