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
