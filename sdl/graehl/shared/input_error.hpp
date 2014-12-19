



#ifndef GRAEHL__SHARED__INPUT_ERROR_HPP
#define GRAEHL__SHARED__INPUT_ERROR_HPP


#ifndef INPUT_ERROR_TELLG
#define INPUT_ERROR_TELLG 1
#endif


#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>

namespace graehl {

#ifndef GRAEHL__ERROR_CONTEXT_CHARS
#define GRAEHL__ERROR_CONTEXT_CHARS 50
#endif
#ifndef GRAEHL__ERROR_PRETEXT_CHARS
#define GRAEHL__ERROR_PRETEXT_CHARS 20
#endif



  using namespace std;
  ostringstream o;
  o << complaint << ": ";
  if (pos == string::npos)
    o << context;
  else
    o << string(context, 0, pos) << "<ERROR HERE>" << string(context, pos);
  throw runtime_error(o.str());
}

template <class C>

  switch (c) {


      return with;
    default:
      return c;
  }
}




}





  char c;
  std::streamoff actual_pretext_chars = 0;
  // if (fstrm * fs = dynamic_cast<fstrm *>(&in)) { // try tell/seek always, -1 return if it fails anyway
  bool ineof = in.eof();
  in.clear();
  std::streamoff before = in.tellg();
  in.unget();
  in.clear();
  // DBP(before);
  if (before >= 0) {
    in.seekg(-(int)prechars, std::ios_base::cur);
    std::streamoff after(in.tellg());
    if (!in) {
      in.clear();
      in.seekg(after = 0);
    }
    actual_pretext_chars = before - after;
  } else
    actual_pretext_chars = 1;  // from unget
  in.clear();
  out << "INPUT ERROR: ";


  out << " (^ marks the read position):\n";
  const unsigned indent = 3;
  output_n(out, '.', indent);
  unsigned ip, ip_lastline = 0;
  for (ip = 0; ip < actual_pretext_chars; ++ip) {
    if (in.get(c)) {
      ++ip_lastline;
#if 1
      out << scrunch_char(c);
#else
      // show newlines
      out << c;
      if (c == '\n') {
        ip_lastline = 0;
      }
#endif
    } else
      break;
  }

  if (ip != actual_pretext_chars)
    out << "<<<WARNING: COULD NOT READ " << prechars << " pre-error characters (only got " << ip << ")>>>";
  for (unsigned i = 0; i < GRAEHL__ERROR_CONTEXT_CHARS; ++i)
    if (in.get(c))
      out << scrunch_char(c);
    else
      break;


  output_n(out, ' ', indent);


  return before >= 0 ? before : -1;
}




  std::ostringstream err;
  err << "Error reading";


  std::streamoff where = show_error_context(in, err);
#if INPUT_ERROR_TELLG

#endif








}




#endif
