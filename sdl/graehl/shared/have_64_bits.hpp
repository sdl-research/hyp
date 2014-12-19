




#ifndef GRAEHL_SHARED__HAVE_64_BITS_HPP
#define GRAEHL_SHARED__HAVE_64_BITS_HPP


#ifndef HAVE_64_BITS

// Check windows


#  define HAVE_64_BITS 1
# else
#  define HAVE_64_BITS 0
# endif

# define HAVE_64_BITS 1
#else
# define HAVE_64_BITS 0
#endif

#endif // HAVE_64_BITS

#endif
