/** \file

    we don't want any flexibility in locale for int<->string, except where we
    explicitly configure the locale (we won't rely on any system configuration
    of a default locale). therefore we can increase performance by hardcoding,
    effectively, atoi and itoa and similar, instead of using streams when we
    sdl::lexical_cast

    include this instead of boost/lexical_cast.hpp (otherwise you probably won't
    get the c-locale speedup)

    note: in xmt/CMakeLists.txt, i've added the
    BOOST_LEXICAL_CAST_ASSUME_C_LOCALE, so this header isn't strictly needed
    except as documentation
*/

#ifndef LEXICALCAST_JG201335_HPP
#define LEXICALCAST_JG201335_HPP
#pragma once

#ifndef BOOST_LEXICAL_CAST_ASSUME_C_LOCALE
# define BOOST_LEXICAL_CAST_ASSUME_C_LOCALE
// use, effectively, atoi, itoa - http://accu.org/index.php/journals/1375

# ifdef BOOST_LEXICAL_CAST_INCLUDED
#  warning for better performance and fewer maybe-uninitialized warnings, please include xmt/LexicalCast.hpp before any other headers that include boost/lexical_cast.hpp
# endif
#endif

#include <graehl/shared/warning_compiler.h>
#if HAVE_GCC_4_6
GCC_DIAG_OFF(maybe-uninitialized)
#endif
#include <boost/lexical_cast.hpp>
#if HAVE_GCC_4_6
    GCC_DIAG_ON(maybe-uninitialized)
#endif

namespace sdl {

using boost::lexical_cast;

}

#endif
