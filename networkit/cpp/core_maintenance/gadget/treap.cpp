#include "treap.h"

#include <random>

#include "../defs.h"

namespace NetworKit{
  namespace gadget {
    Treap::Treap(const count n) {
      ASSERT(0 <= (n_ = n));
      nd_ = std::vector<TreapNode>(n_ + 2);
      // generate random priorities
      std::default_random_engine generator;
      std::uniform_int_distribution<index> distribution(-n_ + 1, n_ - 1);
      for (index i = 0; i < n_; ++i) {
	nd_[i].w = distribution(generator);
      }
      nd_[n_].l = nd_[n_].r = n_;
      nd_[n_].s = 0;
      nd_[n_].w = -n_;
      nd_[n_ + 1].w = n_;
    }
    void Treap::Insert(const index x, const bool f, index& r) {
      if (n_ == r) {
	nd_[x].l = nd_[x].r = n_;
	nd_[x].s = 1;
	r = x;
      } else {
	++nd_[r].s;
	index& c = f ? nd_[r].l : nd_[r].r;
	Insert(x, f, c);
	nd_[c].p = r;
	// the heap property may violate
	if (nd_[c].w > nd_[r].w) {
	  if (f) {
	    RightRotate(r);
	  } else {
	    LeftRotate(r);
	  }
	}
      }
      nd_[r].p = n_;
    }
    void Treap::InsertAfter(const index x, const index y, index& r) {
      Insert(x, true, nd_[y].r);
      nd_[nd_[y].r].p = y;
      index p = y;
      index c = nd_[y].r;
      while (n_ != p) {
	++nd_[p].s;
	index* gp = &r;
	if (n_ != nd_[p].p) {
	  if (p == nd_[nd_[p].p].l) {
	    gp = &nd_[nd_[p].p].l;
	  } else {
	    gp = &nd_[nd_[p].p].r;
	  }
	}
	if (nd_[c].w > nd_[p].w) {
	  if (c == nd_[p].l) {
	    RightRotate(*gp);
	  } else {
	    LeftRotate(*gp);
	  }
	}
	c = *gp;
	p = nd_[c].p;
      }
    }
    void Treap::Delete(const index x, index& r) {
      index y = nd_[x].p;
      while (n_ != y) { // move y up the tree until y's parent is n_
	--nd_[y].s; 
	y = nd_[y].p;
      }
      // while (nd_[x].l != n_ && nd_[x].r != n_) {
      while (nd_[x].l != n_ || nd_[x].r != n_) {	
	// loop until x has some child that is n_
	// std::cerr << "nd_[x].l=" << nd_[x].l << ", nd_[x].r=" << nd_[x].r << ", n_=" <<  n_ << std::endl;
	index* p = &r;
	if (n_ != nd_[x].p) {
	  if (x == nd_[nd_[x].p].l) {
	    p = &nd_[nd_[x].p].l;
	  } else {
	    p = &nd_[nd_[x].p].r;
	  }
	}
	if (nd_[nd_[x].l].w > nd_[nd_[x].r].w) {
	  RightRotate(*p);
	} else {
	  LeftRotate(*p);
	}
	--nd_[*p].s;
      }
      if (nd_[x].p == n_) {
	r = n_;
      } else if (nd_[nd_[x].p].l == x) {
	nd_[nd_[x].p].l = n_;
      } else {
	nd_[nd_[x].p].r = n_;
      }
    }
    index Treap::Merge(const index r1, const index r2) {
      nd_[n_ + 1].l = r1;
      nd_[r1].p = n_ + 1;
      nd_[n_ + 1].r = r2;
      nd_[r2].p = n_ + 1;
      nd_[n_ + 1].p = n_;
      nd_[n_ + 1].s = nd_[r1].s + nd_[r2].s + 1;
      index r = n_ + 1;
      Delete(n_ + 1, r);
      return r;
    }
    index Treap::Rank(const index x) const {
      index rank = nd_[nd_[x].l].s + 1;
      index y = x;
      index p = nd_[y].p;
      while (n_ != p) {
	if (nd_[p].r == y) {
	  rank += nd_[p].s - nd_[y].s;
	}
	y = p;
	p = nd_[p].p;
      }
      return rank;
    }
    index Treap::Select(const index r, const index rank) const {
      ASSERT(rank >= 1 && rank <= nd_[r].s);
      const index local_rank = nd_[nd_[r].l].s + 1;
      if (local_rank == rank) {
	return r;
      } else if (local_rank > rank) {
	return Select(nd_[r].l, rank);
      } else {
	return Select(nd_[r].r, rank - local_rank);
      }
    }
    index Treap::Root(const index x) const {
      index r = x;
      while (n_ != nd_[r].p) {
	r = nd_[r].p;
      }
      return r;
    }
    index Treap::Minimum(const index x) const {
      index m = Root(x);
      while (n_ != nd_[m].l) {
	m = nd_[m].l;
      }
      return m;
    }
    index Treap::Maximum(const index x) const {
      index m = Root(x);
      while (n_ != nd_[m].r) {
	m = nd_[m].r;
      }
      return m;
    }
    index Treap::Size(const index r) const {
      return nd_[r].s;
    }
    void Treap::Check(const index r) const {
      ASSERT(nd_[r].p == n_);
      ASSERT(nd_[n_].l == n_ && nd_[n_].r == n_);
      ASSERT(nd_[n_].s == 0);
      SubCheck(r);
    }
    void Treap::LeftRotate(index& x) {
      const index y = nd_[x].r;
      nd_[x].r = nd_[y].l;
      nd_[y].l = x;
      nd_[y].p = nd_[x].p;
      nd_[nd_[x].r].p = x;
      nd_[x].p = y;
      nd_[y].s = nd_[x].s;
      nd_[x].s = nd_[nd_[x].l].s + nd_[nd_[x].r].s + 1;
      x = y;
    }
    void Treap::RightRotate(index& x) {
      const index y = nd_[x].l;
      nd_[x].l = nd_[y].r;
      nd_[y].r = x;
      nd_[y].p = nd_[x].p;
      nd_[nd_[x].l].p = x;
      nd_[x].p = y;
      nd_[y].s = nd_[x].s;
      nd_[x].s = nd_[nd_[x].l].s + nd_[nd_[x].r].s + 1;
      x = y;
    }
    void Treap::SubCheck(const index x) const {
      if (n_ != nd_[x].l) {
	ASSERT(nd_[nd_[x].l].p == x);
	SubCheck(nd_[x].l);
      }
      if (n_ != nd_[x].r) {
	ASSERT(nd_[nd_[x].r].p == x);
	SubCheck(nd_[x].r);
      }
      ASSERT(nd_[x].s == nd_[nd_[x].l].s + nd_[nd_[x].r].s + 1);
      ASSERT(nd_[x].w >= nd_[nd_[x].l].w && nd_[x].w >= nd_[nd_[x].r].w);
    }
  }  // namespace gadget
}
/*
namespace {
constexpr int n = 1000000;
}  // namespace

int main() {
  gadget::Treap tree(n);
  std::vector<int> roots(100, n);
  std::vector<int> counts(100, 0);
  std::vector<int> numbers(n);

  for (int i = 0; i < n; ++i) {
    numbers[i] = rand() % 100;
    ++counts[numbers[i]];
    tree.Insert(i, rand() % 2, roots[numbers[i]]);
  }
  for (int i = 0; i < 100; ++i) {
    if (roots[i] != n) {
      tree.Check(roots[i]);
      ASSERT(tree.Size(roots[i]) == counts[i]);
    }
  }
  {
    for (int i = 1; i < 100; ++i) {
      roots[0] = tree.Merge(roots[0], roots[i]);
    }
    tree.Check(roots[0]);
    ASSERT(tree.Size(roots[0]) == n);
    printf("tree size: %d\n", tree.Size(roots[0]));
  }
  /star
  for (int i = 0; i < n; ++i) {
    const int b = numbers[i];
    const int nb = rand() % 100;
    if (nb != b) {
      --counts[b];
      ++counts[nb];
      numbers[i] = nb;
      tree.Delete(i, roots[b]);
      tree.Insert(i, rand() % 2, roots[nb]);
    }
  }
  for (int i = 0; i < 100; ++i) {
    if (roots[i] != n) {
      tree.Check(roots[i]);
      ASSERT(tree.Size(roots[i]) == counts[i]);
    }
  }
  for (int i = 0; i < n; ++i) {
    const int b = numbers[i];
    tree.Delete(i, roots[b]);
    --counts[b];
  }
  for (int i = 0; i < 100; ++i) {
    ASSERT(roots[i] == n);
  }
  for (int i = 0; i < n; ++i) {
    tree.Insert(i, false, roots[0]);
  }
  ASSERT(tree.Size(roots[0]) == n);
  for (int i = 0; i < n; ++i) {
    ASSERT(tree.Rank(i, roots[0]) == i + 1);
  }
  for (int i = 0; i < n; ++i) {
    tree.Delete(i, roots[0]);
    tree.InsertAfter(i, (n - 1 + i) % n, roots[0]);
  }
  for (int i = 0; i < n; ++i) {
    ASSERT(tree.Rank(i, roots[0]) == i + 1);
  }
  tree.Check(roots[0]);
  star/
}
*/
