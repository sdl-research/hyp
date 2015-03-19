// Copyright (C) 2000, 2001 Stephen Cleary
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org for updates, documentation, and revision history.

#ifndef POOL__POOLFWD_HPP
#define POOL__POOLFWD_HPP
#pragma once

/*!
  \file
  \brief Forward declarations of all public (non-implemention) classes.
*/


#include <boost/config.hpp> // for workarounds

// std::size_t
#include <cstddef>

namespace Pool {

//
// Location: <Pool/simple_segregated_storage.hpp>
//
template <typename SizeType = std::size_t>
class simple_segregated_storage;

//
// Location: <Pool/pool.hpp>
//
struct default_user_allocator_new_delete;
struct default_user_allocator_malloc_free;

template <typename UserAllocator = default_user_allocator_new_delete>
class pool;

//
// Location: <Pool/object_pool.hpp>
//
template <typename T, typename UserAllocator = default_user_allocator_new_delete>
class object_pool;


}

#endif
