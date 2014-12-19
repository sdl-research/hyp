/** \file

 Your function might return a reference to a member, or it might return a new
 value. Instead of using shared_ptr, just use this.

 auto_ptr like copy/assign - last assigned to / copied to will free on
 destruction
*/

#ifndef MAYBEREF_JG_2014_09_24_HPP
#define MAYBEREF_JG_2014_09_24_HPP
#pragma once

namespace sdl { namespace Util {

template <class Val>
struct MaybeRef {
  typedef Val value_type;

  /**
     maybe take ownership from o.
  */
  MaybeRef(MaybeRef const& o)
      : val_(o.val_)
      , free_(o.free_)
  {
    o.free_ = false;
  }

  /**
     maybe take ownership from o.
  */
  void operator=(MaybeRef const& o)
  {
    val_ = o.val_;
    free_ = o.free_;
    o.free_ = false;
  }

  /**
     val must live as long as anyone uses the MaybeRef to it, and we won't
     delete it
  */
  MaybeRef(value_type & val)
      : val_(&val)
      , free_()
  {}

  /**
     if \param free then delete val when the last-copied MaybeRef to it goes
     away (this is *not* reference counting and so when you assign/copy the
     destination is the only one that maintains the lifetime.
  */
  MaybeRef(value_type * val, bool free = true)
      : val_(val)
      , free_(free)
  {}

  ~MaybeRef() {
    if (free_)
      delete val_;
  }

  value_type *get() const { return val_; }
  value_type *operator -> () const { return val_; }
  value_type & operator*() const { return *val_; }
  value_type *release() {
    free_ = false;
    return val_;
  }

 private:
  value_type *val_;
  mutable bool free_;
};


}}

#endif
