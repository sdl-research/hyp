
















 */







#endif


#include <graehl/shared/show.hpp>
#define DDARY(x) x
DECLARE_DBG_LEVEL(DDARY)
#else
#define DDARY(x)
#endif









#ifndef DEFAULT_DBG_D_ARY_VERIFY_HEAP
#define DEFAULT_DBG_D_ARY_VERIFY_HEAP 0
// this is very slow if enabled
#endif

#ifdef NDEBUG
#define DBG_D_ARY_VERIFY_HEAP 0
#else
#define DBG_D_ARY_VERIFY_HEAP DEFAULT_DBG_D_ARY_VERIFY_HEAP
#endif















#define D_ARY_VERIFY_HEAP DBG_D_ARY_VERIFY_HEAP


#undef D_ARY_HEAP_NULL_INDEX



namespace graehl {

static const std::size_t OPTIMAL_HEAP_ARITY = 4;

/* adapted from boost/graph/detail/d_ary_heap.hpp

  local modifications:

  clear, heapify, append range/container, Size type template arg, reserve constructor arg

  hole+move rather than swap.  note: swap would be more efficient for heavyweight keys, until move ctors exist

  don't set locmap to -1 when removing from heap (waste of time)

  indices start at 0, not 1:
  // unlike arity=2 case, you don't gain anything by having indices start at 1, with 0-based child indices
  // root @1, A=2, children indices m= {0,1}: parent(i)=i/2, child(i, m)=2*i+m
  // root @0: parent(i)=(i-1)/A child(i, n)=i*A+n+1 - can't improve on this except child(i, m)=i*A+m
  (integer division, a/b=floor(a/b), so (i-1)/A = ceil(i/A)-1, or greatest int less than (i/A))

  actually, no need to adjust child index, since child is called only once and inline

  e.g. for A=3 gorn address in tree -> index

  () = root -> 0
  (1) -> 1
  (2) -> 2
  (3) (A) -> 3
  (1,1) -> (1*A+1) = 4
  (1,2) -> (1*A+2) = 5
  (1,3) -> (1*A+3) = 6
  (2,1) -> (2*A+1) = 7
  etc.












 cache-aligned 4-heap speedup over regular 2-heap is 10-80% (for huge heaps, the speedup is more)

 splay/skew heaps are worse than 2heap or aligned 4heap in practice.



 #define D_ARY_BYTES_OUT_OF_CACHE 0x1000000





 */

//
//=======================================================================
// Copyright 2009 Trustees of Indiana University
// Authors: Jeremiah J. Willcock, Andrew Lumsdaine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
//

#include <vector>
#include <cstddef>
#include <algorithm>
#include <utility>
#include <cassert>
#include <boost/static_assert.hpp>
#include <boost/shared_array.hpp>


// D-ary heap using an indirect compare operator (use identity_property_map
// as DistanceMap to get a direct compare operator).  This heap appears to be
// commonly used for Dijkstra's algorithm for its good practical performance
// on some platforms; asymptotically, it's not optimal; it has an O(lg N) decrease-key
// operation, which is (amortized) constant time on a relaxed heap or fibonacci heap.  The
// implementation is mostly based on the binary heap page on Wikipedia and
// online sources that state that the operations are the same for d-ary
// heaps.  This code is not based on the old Boost d-ary heap code.
//
// - d_ary_heap_indirect is a model of UpdatableQueue as is needed for
//   dijkstra_shortest_paths.
//
// - Value must model Assignable.
// - Arity must be at least 2 (optimal value appears to be 4, both in my and
//   third-party experiments).
// - IndexInHeapMap must be a ReadWritePropertyMap from Value to
//   Container::size_type (to store the index of each stored value within the
//   heap for decrease-key aka update).
// - DistanceMap must be a ReadablePropertyMap from Value to something
//   (typedef'ed as distance_type).
// - Compare must be a BinaryPredicate used as a less-than operator on
//   distance_type.
// - Container must be a random-access, contiguous container (in practice,
//   the operations used probably require that it is std::vector<Value>).
//



class d_ary_heap_indirect {
  BOOST_STATIC_ASSERT(Arity >= 2);

 public:
  typedef Container container_type;
  typedef Size size_type;
  typedef Value value_type;
  typedef typename Container::const_iterator const_iterator;
  typedef const_iterator iterator;
  // The distances being compared using better and that are stored in the
  // distance map
  typedef typename boost::property_traits<DistanceMap>::value_type distance_type;





  }
  /* Implicit copy constructor */
  /* Implicit assignment operator */

  template <class C>
  void append_heapify(C const& c) {
    data.reserve(data.size() + c.size());
    append_heapify(c.begin(), c.end());
  }

  template <class I>
  void append_heapify(I begin, I end) {
    std::size_t s = data.size();
    data.insert(data.end(), begin, end);
    set_index_in_heap(s);  // allows contains() and heapify() to function properly
    heapify();
  }

  template <class C>
  void append_push(C const& c) {
    data.reserve(data.size() + c.size());
    append_push(c.begin(), c.end());
  }



  template <class I>
  void append_push(I begin, I end) {

  }

  template <class C>
  void append(C const& c) {
    if (D_ARY_APPEND_ALWAYS_PUSH || data.size() >= c.size() / 2)
      append_push(c);
    else
      append_heapify(c);
  }



  template <class I>
  void append(I begin, I end) {
    if (D_ARY_APPEND_ALWAYS_PUSH || data.size() >= 0x10000)
      append_push(begin, end);
    else
      append_heapify(begin, end);
  }

  template <class V>
  void add_unsorted(V const& v) {
    put(index_in_heap, v, data.size());  // allows contains() and heapify() to function properly
    data.push_back(v);
  }







    put(index_in_heap, v, data.size());  // allows contains() and heapify() to function properly
    data.push_back(v);
  }



  // for debugging, please


  // call heapify yourself after.



  // from bottom of heap tree up, turn that subtree into a heap by adjusting the root down




  void set_index_in_heap(std::size_t i = 0) {
    for (std::size_t e = data.size(); i < e; ++i) {

      set(index_in_heap, data[i], i);
    }
  }

  /*
The basic building block of "heapify" is a simple procedure which I'd
call "preserve_heap_property_down". It looks at a node that is not a leaf, and among
that non-leaf node and its two children, which is largest. Then it
swap that largest node to the top, so that the node becomes the
largest among the three---satisfy the "heap property". Clearly,
constant time.

To heapify the whole tree, we will heapify it from the bottom (so
"bottom up"). Heapifying a node at the "bottom" (i.e., next to a
leaf) is trivial; heapifying a node that is not bottom might cause the
heap property of nodes below to be violated. In that case we will
have to heapify the node below as well, which might cause a node one
more level below to violate heap property. So to heapify a node at
level n, we might need to call preserve_heap_property_down (height-1) times.

Now we show that a whole run of heapify of a tree with n=2^k-1 nodes
will never call preserve_heap_property_down more than n times. We do it by finding a
way to charge the calls to preserve_heap_property_down so that each node is never
charged more than once. Note that we simplify things by always
considering full binary trees (otherwise, the claim has to be a bit
more complicated).

Let's consider the bottom layer. To heapify a node with two children,
we need one call to preserve_heap_property_down. It is charged to left leaf. So we
have this:

After heapify O
/ \
X O

where O shows a node that is not charged yet, and X show a node which
is already charged. Once all the next-to-bottom nodes have been
heapified, we start to heapify the next-to-next-to-bottom nodes.
Before heapifying a node there, we have

Before heapify O
_/ \_
O O
/ \ / \
X O X O

To heapify this node, we might need two calls to preserve_heap_property_down. We
charge these two calls to the two uncharged nodes of the left subtree.
So we get:

After heapify O
_/ \_
X O
/ \ / \
X X X O

So none of the nodes is charged more than once, and we still have some
left for the next level, where before heapify the picture is:

Before heapify O
____/ \____
O O
_/ \_ _/ \_
X O X O
/ \ / \ / \ / \
X X X O X X X O

Heapifying at this level requires at most 3 calls to preserve_heap_property_down,
which are charged again to the left branch. So after that we get

After heapify O
____/ \____
X O
_/ \_ _/ \_
X X X O
/ \ / \ / \ / \
X X X X X X X O

We note a pattern: the path towards the right-most leaf is never
charged. When heapifying a level, one of the branches will always
have enough uncharged nodes to pay for the "expensive" heapify at the
top, while the other branch will still be uncharged to keep the
pattern. So this pattern is maintained until the end of the heapify
procedure, making the number of steps to be at most n-k = 2^k - k - 1.
This is definitely linear to n.
   */
  void heapify() {
    EIFDBG(DDARY, 1, SHOWM1(DDARY, "heapify", data.size()));


      --i;
      EIFDBG(DDARY, 2, SHOWM1(DDARY, "heapify", i));
      preserve_heap_property_down(i);
    }
    verify_heap();
  }











  void clear() {
#if D_ARY_TRACK_OUT_OF_HEAP
    using boost::put;
    for (typename Container::iterator i = data.begin(), e = data.end(); i != e; ++i)
      put(index_in_heap, *i, (size_type)D_ARY_HEAP_NULL_INDEX);
#endif
    data.clear();
  }




  void push(const Value& v) {
    if (D_ARY_PUSH_GRAEHL) {
      size_type i = data.size();
      data.push_back(Value());  // (hoping default construct is cheap, construct-copy inline)
      preserve_heap_property_up(v, i);  // we don't have to recopy v, or init index_in_heap
    } else {
      size_type index = data.size();
      data.push_back(v);
      using boost::put;
      put(index_in_heap, v, index);
      preserve_heap_property_up(index);
    }
    verify_heap();
  }













  void pop() {
    using boost::put;

    if (data.size() != 1) {
      if (D_ARY_POP_GRAEHL) {
        preserve_heap_property_down(data.back(), 0, data.size() - 1);
        data.pop_back();
      } else {
        data[0] = data.back();
        put(index_in_heap, data[0], 0);
        data.pop_back();
        preserve_heap_property_down();
      }
      verify_heap();
    } else {
      data.pop_back();
    }
  }

  // This function assumes the key has been improved
  // (distance has become smaller, so it may need to rise toward top().
  // i.e. decrease-key in a min-heap
  void update(const Value& v) {
    using boost::get;
    size_type index = get(index_in_heap, v);
    preserve_heap_property_up(v, index);
    verify_heap();
  }

  // return true if improved.
  bool maybe_improve(const Value& v, distance_type dbetter) {
    using boost::get;
    if (better(dbetter, get(distance, v))) {
      preserve_heap_property_up_dist(v, dbetter);
      return true;
    }
    return false;
  }


  distance_type second_best(distance_type null = 0) const {
    if (data.size() < 2) return null;
    int m = std::min(data.size(), Arity + 1);
    //      if (m>=Arity) m=Arity+1;
    distance_type b = get(distance, data[1]);
    for (int i = 2; i < m; ++i) {
      distance_type d = get(distance, data[i]);

    }
    return b;
  }








  inline bool contains(const Value& v, size_type i) const {

    size_type sz = data.size();
    EIFDBG(DDARY, 2, SHOWM2(DDARY, "d_ary_heap contains", i, data.size()));


  }


  inline bool contains(const Value& v) const {
    using boost::get;
    return contains(v, get(index_in_heap, v));
  }

  void push_or_update(const Value& v) { /* insert if not present, else update */
    using boost::get;
    size_type index = get(index_in_heap, v);
    if (D_ARY_PUSH_GRAEHL) {
      if (contains(v, index))
        preserve_heap_property_up(v, index);
      else
        push(v);
    } else {
      if (!contains(v, index)) {
        index = data.size();
        data.push_back(v);
        using boost::put;
        put(index_in_heap, v, index);
      }
      preserve_heap_property_up(index);
    }
    verify_heap();
  }

 private:
  Better better;
  Container data;
  DistanceMap distance;
  IndexInHeapPropertyMap index_in_heap;
  Equal equal;

  // Get the parent of a given node in the heap


  // Get the child_idx'th child of a given node; 0 <= child_idx < Arity
  static inline size_type child(size_type index, std::size_t child_idx) {
    return index * Arity + child_idx + 1;
  }

  // Swap two elements in the heap by index, updating index_in_heap
  inline void swap_heap_elements(size_type index_a, size_type index_b) {
    using std::swap;
    Value value_a = data[index_a];
    Value value_b = data[index_b];
    data[index_a] = value_b;
    data[index_b] = value_a;
    using boost::put;
    put(index_in_heap, value_a, index_b);
    put(index_in_heap, value_b, index_a);
  }

  inline void move_heap_element(Value const& v, size_type ito) {
    using boost::put;
    put(index_in_heap, v, ito);
    data[ito] = v;  // todo: move assign?
  }

  // Verify that the array forms a heap; commented out by default
  void verify_heap() const {
// This is a very expensive test so it should be disabled even when
// NDEBUG is not defined
#if D_ARY_VERIFY_HEAP
    using boost::get;
    for (size_t i = 1; i < data.size(); ++i) {
      if (better(get(distance, data[i]), get(distance, data[parent(i)]))) {
        assert(!"Element is smaller than its parent");
      }
      if (get(index_in_heap, data[i]) != i) {
        assert(!"Element is where its index_in_heap doesn't say it is.");
      }
    }
#endif
  }




  inline void preserve_heap_property_up(Value const& currently_being_moved, size_type index) {
    using boost::get;
    preserve_heap_property_up(currently_being_moved, index, get(distance, currently_being_moved));
  }





  inline void preserve_heap_property_up_set_dist(Value const& currently_being_moved, distance_type dbetter) {
    using boost::get;
    using boost::put;
    put(distance, currently_being_moved, dbetter);
    preserve_heap_property_up(currently_being_moved, get(index_in_heap, currently_being_moved), dbetter);
    verify_heap();
  }




    using boost::put;
    using boost::get;
    if (D_ARY_UP_GRAEHL) {
      for (;;) {
        if (index == 0) break;  // Stop at root
        size_type parent_index = parent(index);
        Value const& parent_value = data[parent_index];
        if (better(currently_being_moved_dist, get(distance, parent_value))) {
          move_heap_element(parent_value, index);
          index = parent_index;
        } else {
          break;  // Heap property satisfied
        }
      }
      // finish "swap chain" by filling hole w/ currently_being_moved



    } else {
      put(index_in_heap, currently_being_moved, index);

      preserve_heap_property_up(index);
    }
  }

  // Starting at a node, move up the tree swapping elements to preserve the
  // heap property.  doesn't actually use swap; uses hole
  void preserve_heap_property_up(size_type index) {
    using boost::get;
    if (index == 0) return;  // Do nothing on root
    if (D_ARY_UP_GRAEHL) {
      Value copyi = data[index];
      preserve_heap_property_up(copyi, index);
      return;
    }
    size_type orig_index = index;
    size_type num_levels_moved = 0;
    // The first loop just saves swaps that need to be done in order to avoid
    // aliasing issues in its search; there is a second loop that does the
    // necessary swap operations
    Value currently_being_moved = data[index];

    for (;;) {
      if (index == 0) break;  // Stop at root
      size_type parent_index = parent(index);
      Value parent_value = data[parent_index];
      if (better(currently_being_moved_dist, get(distance, parent_value))) {
        ++num_levels_moved;
        index = parent_index;
        continue;
      } else {
        break;  // Heap property satisfied
      }
    }
    // Actually do the moves -- move num_levels_moved elements down in the
    // tree, then put currently_being_moved at the top
    index = orig_index;
    using boost::put;
    for (size_type i = 0; i < num_levels_moved; ++i) {
      size_type parent_index = parent(index);
      Value parent_value = data[parent_index];
      put(index_in_heap, parent_value, index);
      data[index] = parent_value;
      index = parent_index;
    }
    data[index] = currently_being_moved;
    put(index_in_heap, currently_being_moved, index);
    verify_heap();
  }


  // From the root, swap elements (each one with its smallest child) if there







    //// hole at index - currently_being_moved to be put here when we find the final hole spot


    using boost::get;
    distance_type currently_being_moved_dist = get(distance, currently_being_moved);
    Value* data_ptr = &data[0];
    for (;;) {
      size_type first_child_index = child(index, 0);
      if (first_child_index >= heap_size) break; /* No children */






      // begin find best child index/distance


      distance_type smallest_child_dist = get(distance, child_base_ptr[smallest_child_index]);
#undef D_ARY_MAYBE_IMPROVE_CHILD_I


    distance_type i_dist = get(distance, child_base_ptr[i]); \
    if (better(i_dist, smallest_child_dist)) {               \
      smallest_child_index = i;                              \
      smallest_child_dist = i_dist;                          \


      if (first_child_index + Arity <= heap_size) {





        }
      } else {


        }
      }
      // end: know best child

      if (better(smallest_child_dist, currently_being_moved_dist)) {
        // instead of swapping, move.
        move_heap_element(child_base_ptr[smallest_child_index], index);  // move up
        index = first_child_index + smallest_child_index;  // descend - hole is now here
      } else {
        move_heap_element(currently_being_moved, index);  // finish "swap chain" by filling hole
        break;
      }
    }
    verify_heap();
  }

  inline void preserve_heap_property_down(size_type i) {
    EIFDBG(DDARY, 3, SHOWM3(DDARY, "preserve_heap_property_down", i, data[i], data.size()));
    preserve_heap_property_down(data[i], i, data.size());
  }







  // moves what's at root downwards if needed
  void preserve_heap_property_down() {
    using boost::get;
    if (data.empty()) return;
    if (D_ARY_DOWN_GRAEHL) {  // this *should* be more efficient because i avoid swaps.
      Value copy0 = data[0];
      preserve_heap_property_down(copy0, 0, data.size());
      return;
    }
    size_type index = 0;
    Value currently_being_moved = data[0];

    size_type heap_size = data.size();
    Value* data_ptr = &data[0];
    for (;;) {
      size_type first_child_index = child(index, 0);
      if (first_child_index >= heap_size) break; /* No children */
      Value* child_base_ptr = data_ptr + first_child_index;
      size_type smallest_child_index = 0;
      distance_type smallest_child_dist = get(distance, child_base_ptr[smallest_child_index]);
      if (first_child_index + Arity <= heap_size) {


        }
      } else {


        }
      }
      if (better(smallest_child_dist, currently_being_moved_dist)) {
        swap_heap_elements(smallest_child_index + first_child_index, index);
        index = smallest_child_index + first_child_index;
        continue;
      } else {
        break;  // Heap property satisfied
      }
    }
    verify_heap();
  }
};




#endif
