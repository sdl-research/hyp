// Copyright 2014-2015 SDL plc
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/**

   boost object_pool in Pool namespace with free(ptr) O(1) instead of O(n)

   (actually O(lg n) amortized; free list is sorted on ~object_pool, but only if you freed individual items)

   see https://github.com/graehl/boost/tree/object_pool-constant-time-free for a version that overwrites the (unmaintained) boost::object_pool

   ---

   from https://svn.boost.org/svn/boost/sandbox/pool rev 83408.

   necessary because boost trunk pool has no maintainer (since 2002)

   changed slightly by graehl:

   I reverted their amendment of the original pool interface (which allowed freeing
   of null pointers), and their optimization for "when all objects were freed,
   don't waste time scanning for unfreed objects" - if you really will free
   everything, don't use an object_pool!

   I also tuned default block size to fit (with pool and TBB allocator overhead)
   8kb. This is important because the old default of 32*sizeof(object) for
   extremely large objects might leave significant wasted space in a TBB 16kb slab.

   the default number of objects to allocate is automatically tuned to fill
   POOL_MALLOC_BLOCK_TARGET with the boost::pool overhead in
   mind. POOL_MALLOC_BLOCK_TARGET will be set so that we can fit two blocks on a
   TBB malloc slab (which is 16kb)

   ---

   advice:

   do *not* explicitly destroy everything you've allocated; it's slower than
   letting the destructor do it, or else use a raw Pool::pool and handle
   construct/destruct yourself, because in order to keep track of what needs
   automatic destructor calls, there's an O(m+n*lg n) cost, where m is high
   water mark of total objects allocated, and n is the size of the free list (so
   if you keep close to peek occupancy, it's actually *faster*).

   ----

   TODO: i believe i can make this approx O(m) with a little up-front overhead,
   for objects of larger than sizeof(void *):

    1. after destroy and on fresh segregation of a new never-allocated block,
   write a constant random-like magic number right after the free-list pointer.

    2. then on object_pool purge_memory, scan through the O(m) (object or
   freelist+magic), creating (in a temporary vector) a list of pointers to
   everything matching the magic number. you can destroy any non-magic (must be
   live object) right now.

    3. iterate over the free list, clearing the magic number

    4. iterate again over the magic-matching objects in 2 (note: the list of
    magic-matched objects was actually optional). these are objects that
    happened to (unfortunately) match the magic number. destroy them now

   whether this is actually faster is an empirical question, but i guess that
   the size of the false-magic-number match set is nearly 0 on average. so the
   extra cost would mostly be in marking the magic number in newly acquired
   simple_segregated_storage and on destroy.

   the current approach is faster if you have very many construct/destroy in a
   steady state (the amount held grows slowly and then asymptotes). in this case
   m would be large but n, the free list size, would be small. the current
   approach has minimal per-construct/destroy overhead (though writing a magic
   number is pretty cheap)

   a bitvector approach (where the bitvector is allocated only on destroying an
   object_pool) would work if we modified the pool/simple_segregated_storage to
   give us a global index across all blocks, or a way to associate a separate
   vector w/ each block. magic numbers are likely faster, though.

   ---
*/

// Copyright (C) 2000, 2001 Stephen Cleary
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org for updates, documentation, and revision history.

#ifndef POOL__OBJECT_POOL_HPP
#define POOL__OBJECT_POOL_HPP
#pragma once
/*!
\file
\brief  Provides a template type boost::object_pool<T, UserAllocator>
that can be used for fast and efficient memory allocation of objects of type T.
It also provides automatic destruction of non-deallocated objects.
*/

#include <Pool/pool.hpp>

// The following code will be put into Boost.Config in a later revision
#if defined(BOOST_MSVC) || defined(__KCC)
# define BOOST_NO_TEMPLATE_CV_REF_OVERLOADS
#endif

// The following code might be put into some Boost.Config header in a later revision
#ifdef __BORLANDC__
# pragma option push -w-inl
#endif

// There are a few places in this file where the expression "this->m" is used.
// This expression is used to force instantiation-time name lookup, which I am
//   informed is required for strict Standard compliance.  It's only necessary
//   if "m" is a member of a base class that is dependent on a template
//   parameter.
// Thanks to Jens Maurer for pointing this out!

namespace Pool {

/*! \brief A template class
that can be used for fast and efficient memory allocation of objects.
It also provides automatic destruction of non-deallocated objects.

\details

<b>T</b> The type of object to allocate/deallocate.
T must have a non-throwing destructor.

<b>UserAllocator</b>
Defines the allocator that the underlying Pool will use to allocate memory from the system.
See <a href="boost_pool/pool/pooling.html#boost_pool.pool.pooling.user_allocator">User Allocators</a> for details.

Class object_pool is a template class
that can be used for fast and efficient memory allocation of objects.
It also provides automatic destruction of non-deallocated objects.

When the object pool is destroyed, then the destructor for type T
is called for each allocated T that has not yet been deallocated. O(N).

Whenever an object of type ObjectPool needs memory from the system,
it will request it from its UserAllocator template parameter.
The amount requested is determined using a doubling algorithm;
that is, each time more system memory is allocated,
the amount of system memory requested is doubled.
Users may control the doubling algorithm by the parameters passed
to the object_pool's constructor.
*/

template <typename T, typename UserAllocator>
class object_pool: protected pool<UserAllocator>
{ //!
  public:
    typedef T element_type; //!< ElementType
    typedef UserAllocator user_allocator; //!<
    typedef typename pool<UserAllocator>::size_type size_type; //!<   pool<UserAllocator>::size_type
    typedef typename pool<UserAllocator>::difference_type difference_type; //!< pool<UserAllocator>::difference_type

  protected:
    //! \return The underlying boost:: \ref pool storage used by *this.
    pool<UserAllocator> & store()
    {
      return *this;
    }
    //! \return The underlying boost:: \ref pool storage used by *this.
    const pool<UserAllocator> & store() const
    {
      return *this;
    }

    // for the sake of code readability :)
    static void * & nextof(void * const ptr)
    { //! \returns The next memory block after ptr (for the sake of code readability :)
      return *(static_cast<void **>(ptr));
    }

  public:
    explicit object_pool(const size_type arg_next_size = 0, const size_type arg_max_size = 0)
    :
    pool<UserAllocator>(sizeof(T), arg_next_size, arg_max_size)
    { //! Constructs a new (empty by default) ObjectPool.
      //! \param next_size Number of chunks to request from the system the next time that object needs to allocate system memory
      //!   This parameter may be 0 (the default), in which case we try to allocate as close as possible to POOL_MALLOC_BLOCK_TARGET
      //! \pre next_size != 0.
      //! \param max_size Maximum number of chunks to ever request from the system - this puts a cap on the doubling algorithm
      //! used by the underlying pool.
    }

    ~object_pool()
    {
      purge_memory();
    }

    bool release_memory()
    {
      return pool<UserAllocator>::release_memory();
    }

    bool purge_memory();

    // Returns 0 if out-of-memory.
    element_type * malloc BOOST_PREVENT_MACRO_SUBSTITUTION()
    { //! Allocates memory that can hold one object of type ElementType.
      //!
      //! If out of memory, returns 0.
      //!
      //! Amortized O(1).
      return static_cast<element_type *>(store().malloc BOOST_PREVENT_MACRO_SUBSTITUTION());
    }

    void free BOOST_PREVENT_MACRO_SUBSTITUTION(element_type * const chunk)
    { //! De-Allocates memory that holds a chunk of type ElementType.
      //!
      //!  Note that chunk may be 0.\n
      //!
      //! Note that the destructor for p is not called. O(N).
      store().free BOOST_PREVENT_MACRO_SUBSTITUTION(chunk);
    }

    bool is_from(element_type * const chunk) const
    { /*! \returns true  if chunk was allocated from *this or
      may be returned as the result of a future allocation from *this.

      Returns false if chunk was allocated from some other pool or
      may be returned as the result of a future allocation from some other pool.

      Otherwise, the return value is meaningless.

      \note This function may NOT be used to reliably test random pointer values!
    */
      return store().is_from(chunk);
    }

    size_type get_size() const
    {
      return store().get_size();
    }

    element_type * construct()
    { //! \returns A pointer to an object of type T, allocated in memory from the underlying pool
      //! and default constructed.  The returned objected can be freed by a call to \ref destroy.
      //! Otherwise the returned object will be automatically destroyed when *this is destroyed.
      element_type * const ret = (malloc)();
      if (ret == 0)
        return ret;
      try { new (ret) element_type(); }
      catch (...) { (free)(ret); throw; }
      return ret;
    }


#if defined(BOOST_DOXYGEN)
    template <class Arg1, ... class ArgN>
    element_type * construct(Arg1&, ... ArgN&)
    {
       //! \returns A pointer to an object of type T, allocated in memory from the underlying pool
       //! and constructed from arguments Arg1 to ArgN.  The returned objected can be freed by a call to \ref destroy.
       //! Otherwise the returned object will be automatically destroyed when *this is destroyed.
       //!
       //! \note Since the number and type of arguments to this function is totally arbitrary, a simple system has been
       //! set up to automatically generate template construct functions. This system is based on the macro preprocessor
       //! m4, which is standard on UNIX systems and also available for Win32 systems.\n\n
       //! detail/pool_construct.m4, when run with m4, will create the file detail/pool_construct.ipp, which only defines
       //! the construct functions for the proper number of arguments. The number of arguments may be passed into the
       //! file as an m4 macro, NumberOfArguments; if not provided, it will default to 3.\n\n
       //! For each different number of arguments (1 to NumberOfArguments), a template function is generated. There
       //! are the same number of template parameters as there are arguments, and each argument's type is a reference
       //! to that (possibly cv-qualified) template argument. Each possible permutation of the cv-qualifications is also generated.\n\n
       //! Because each permutation is generated for each possible number of arguments, the included file size grows
       //! exponentially in terms of the number of constructor arguments, not linearly. For the sake of rational
       //! compile times, only use as many arguments as you need.\n\n
       //! detail/pool_construct.bat and detail/pool_construct.sh are also provided to call m4, defining NumberOfArguments
       //! to be their command-line parameter. See these files for more details.
    }
#else
// Include automatically-generated file for family of template construct() functions.
// Copy .inc renamed .ipp to conform to Doxygen include filename expectations, PAB 12 Jan 11.
// But still get Doxygen warning:

  // Warning: include file boost/pool/detail/pool_construct.ipp
// not found, perhaps you forgot to add its directory to INCLUDE_PATH?
// But the file IS found and referenced OK, but cannot view code.
// This seems because not at the head of the file
// But if moved this up, Doxygen is happy, but of course it won't compile,
// because the many constructors *must* go here.

#ifndef BOOST_NO_TEMPLATE_CV_REF_OVERLOADS
#   include <boost/pool/detail/pool_construct.ipp>
#else
#   include <boost/pool/detail/pool_construct_simple.ipp>
#endif
#endif
    void destroy(element_type * const chunk)
    { //! Destroys an object allocated with \ref construct.
      //!
      //! Equivalent to:
      //!
      //! p->~ElementType(); this->free(p);
      //!
      //! \pre p must have been previously allocated from *this via a call to \ref construct.
      chunk->~T();
      (free)(chunk);
    }

    size_type get_next_size() const
    { //! \returns The number of chunks that will be allocated next time we run out of memory.
      return store().get_next_size();
    }

    void set_next_size(const size_type x)
    { //! Set a new number of chunks to allocate the next time we run out of memory.
      //! \param x wanted next_size (must not be zero).
      store().set_next_size(x);
    }

    size_type get_max_size() const
    { //! \returns max_size.
      return pool<UserAllocator>::get_max_size();
    }

    void set_max_size(const size_type nmax_size)
    { //! Set max_size.
      pool<UserAllocator>::set_max_size(nmax_size);
    }
};

template <typename T, typename UserAllocator>
bool object_pool<T, UserAllocator>::purge_memory()
{
#ifndef BOOST_POOL_VALGRIND
  // handle trivial case of invalid list.
  if (!this->list.valid())
    return false;

  // delete all objects that are not freed
    // sort store
    store().order();

    details::PODptr<size_type> iter = this->list;
    details::PODptr<size_type> next = iter;

    // Start 'freed_iter' at beginning of free list
    void * freed_iter = this->first;

    const size_type partition_size = this->alloc_size();

    do
    {
      // increment next
      next = next.next();

      // Iterate 'i' through all chunks in the memory block.
      for (char * i = iter.begin(); i != iter.end(); i += partition_size)
      {
        // If this chunk is free,
        if (i == freed_iter)
        {
          // Increment freed_iter to point to next in free list.
          freed_iter = nextof(freed_iter);

          // Continue searching chunks in the memory block.
          continue;
        }

        // This chunk is not free (allocated), so call its destructor,
        static_cast<T *>(static_cast<void *>(i))->~T();
        // and continue searching chunks in the memory block.
      }

      // increment iter.
      iter = next;
    } while (iter.valid());
#else
  // destruct all used elements:
  for(std::set<void*>::iterator pos = this->used_list.begin(); pos != this->used_list.end(); ++pos)
  {
    static_cast<T*>(*pos)->~T();
  }
#endif

  // Call inherited purge function
  return pool<UserAllocator>::purge_memory();
}

}


// The following code might be put into some Boost.Config header in a later revision
#ifdef __BORLANDC__
# pragma option pop
#endif

#endif
