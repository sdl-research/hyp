// Copyright 2014-2015 SDL plc
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/*
  Forall.hpp - define a 'forall' in terms of C++11 for range or
  BOOST_FORALL. TODO: remove once everyone is using C++11 for loops

  we can't use 'forall' safely because that's a namespace inside boost

*/

#ifndef SDL_UTIL_FORALL_HPP_
#define SDL_UTIL_FORALL_HPP_
#pragma once


#include <boost/foreach.hpp>

// TODO: this will probably be unnecessary in boost 1.49 and later - see
// https://svn.boost.org/trac/boost/ticket/6131
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>


// Eclipse CDT parser does not understand BOOST_FOREACH,
// see https://bugs.eclipse.org/bugs/show_bug.cgi?id=332278.
#ifdef __CDT_PARSER__
#ifdef forall
#undef forall
#endif
#define forall (a, b) for (a : b)
#define SDL_UTIL_forall(a, b) for (a : b)
#else
#define SDL_UTIL_forall(a, b) for (a : b)
#define forall SDL_UTIL_forall
#endif

#endif
