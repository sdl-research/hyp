





inline double random0n(double n)  // random from [0..n)
{
  return n * random01();
}


inline double random_pos_fraction()  // returns uniform random number on (0..1]
{
#ifdef USE_STD_RAND

#else
  return 1. - random01();
#endif
}

template <class V1, class V2>









  // correct against bias (which is worse when limit is almost RAND_MAX)




  return r % limit;
#else

#endif
}


  return random_less_than(2);
}






  return random_less_than(limit + 1);
}


#define GRAEHL_RANDOM__NLETTERS 26
// works for only if a-z A-Z and 0-9 are contiguous
inline char random_alpha() {

  return (r < GRAEHL_RANDOM__NLETTERS) ? 'a' + r : ('A' - GRAEHL_RANDOM__NLETTERS) + r;
}

inline char random_alphanum() {



             : ('0' - GRAEHL_RANDOM__NLETTERS * 2) + r;
}
#undef GRAEHL_RANDOM__NLETTERS

inline std::string random_alpha_string(unsigned len) {
  boost::scoped_array<char> s(new char[len + 1]);
  char* e = s.get() + len;
  *e = '\0';

  return s.get();
}

// P(*It) = double probability (unnormalized).
template <class It, class P>




  double choice = sum * random01();
  for (It i = begin;;) {
    choice -= p(*i);
    It r = i;
    ++i;
    if (choice < 0 || i == end) return r;
  }
  return begin;  // unreachable
}

// P(*It) = double probability (unnormalized).
template <class Sum, class It, class P>




  double choice = random01();
  for (It i = begin;;) {
    choice -= p(*i) / sum;
    It r = i;
    ++i;
    if (choice < 0 || i == end) return r;
  }
  return begin;  // unreachable
}

// as above but already normalized
template <class It, class P>

  double sum = 0.;
  double choice = random01();
  for (It i = begin;;) {
    sum += p(*i);
    It r = i;
    ++i;
    if (sum > choice || i == end) return r;
  }
  return begin;  // unreachable
}


template <class It>

  using std::swap;
  size_t N = end - begin;
  for (size_t i = 0; i < N; ++i) {
    swap(*(begin + i), *(begin + random_up_to(i)));
  }
}

template <class V>

  using std::swap;
  size_t N = vec.size();
  for (size_t i = 0; i < N; ++i) {
    swap(vec[i], vec[random_up_to(i)]);
  }

