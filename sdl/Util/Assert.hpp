// Copyright 2014 SDL plc
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

#ifndef SDL_UTIL_ASSERT_HPP
#define SDL_UTIL_ASSERT_HPP
#pragma once

#ifndef BOOST_DISABLE_ASSERTS
#ifdef NDEBUG
#define BOOST_DISABLE_ASSERTS 1
#endif
#endif
#define BOOST_ENABLE_ASSERT_HANDLER 1

#include <boost/assert.hpp>
#include <sdl/Exception.hpp>

VERBOSE_EXCEPTION_DECLARE(AssertionFailure)

#define SDL_ASSERT(expr) BOOST_ASSERT(expr)
#define SDL_ASSERT_MSG(expr, msg) BOOST_ASSERT_MSG(expr, msg)

#endif
