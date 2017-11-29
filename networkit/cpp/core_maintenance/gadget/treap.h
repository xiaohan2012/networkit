// The @Treap class implements the treap structure.
#ifndef CORE_GADGET_TREAP_H_
#define CORE_GADGET_TREAP_H_

#include <vector>

namespace NetworKit{
  namespace gadget {
    class Treap final {
    public:
      explicit Treap(const count n);

      void Insert(const index x, const bool f, index& r);
      void InsertAfter(const index x, const index y, index& r);
      void Delete(const index x, index& r);
      index Merge(const index r1, const index r2);
      index Rank(const index x) const;
      index Select(const index r, const index rank) const;
      index Root(const index x) const;
      index Minimum(const index x) const;
      index Maximum(const index x) const;
      index Size(const index r) const;
      void Check(const index r) const;

    private:
      struct TreapNode final {
	index p;  // the parent
	index l;  // the left child
	index r;  // the right child
	index s;  // the size of the subtree rooted at this node
	index w;  // the priority
      };

      void LeftRotate(index& x);
      void RightRotate(index& x);
      void SubCheck(const index x) const;

      index n_;
      std::vector<TreapNode> nd_;
    };
  }  // namespace gadget
}
#endif
