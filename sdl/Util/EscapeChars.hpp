




#ifndef LWUTIL_QI_KARMA_SYMBOLS_HPP
#define LWUTIL_QI_KARMA_SYMBOLS_HPP


#include <utility>
#include <vector>





























  typedef std::pair<char, char const*> Esc;
  typedef std::vector<Esc> Escs;
  Escs escs;
  EscapeChars& add() { return *this; }
  EscapeChars& add(char c, char const* t) {
    escs.push_back(Esc(c, t));
    return *this;
  }

  template <class QiSym>


  }
  template <class KarmaSym>


  }
};




#endif
