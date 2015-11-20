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
/** \file

    word wrap. for documentation, -h usage, etc.
*/

#ifndef SDL_UTIL_FORMATTEDOSTREAM_HPP
#define SDL_UTIL_FORMATTEDOSTREAM_HPP
#pragma once

#include <cctype>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <sdl/SharedPtr.hpp>

namespace sdl {
namespace Util {

/**
   Formatted output stream for pretty-printing text.

   Taken from http://stackoverflow.com/a/5280960/60628

   Example usage:

   \code
   const std::string text(
      "Friends, Romans, countrymen, lend me your ears; I come to bury Caesar, "
      " not to praise him.  The evil that men do lives after them; The good "
      "is oft interred with their bones; So let it be with Caesar. ReallyLong"
      "WordThatWontFitOnOneLineBecauseItIsSoFreakinLongSeriouslyHowLongIsThis"
      "Word");
   std::copy(text.begin(), text.end(),
            FormattedOstreamIterator(std::cerr, "    ", 40));

   \endcode
   will print:

   \code
    Friends, Romans, countrymen, lend me
    your ears; I come to bury Caesar, not to
    praise him. The evil that men do lives
   [...]
 */
class FormattedOstreamIterator : public std::iterator<std::output_iterator_tag, char, void, void, void> {
 public:
  FormattedOstreamIterator() {}

  /** if maxFirstLineLength, then omit first line's prefix and use that maximum line length for first line,
   * then  max_line_length */
  FormattedOstreamIterator(std::ostream& os, std::string line_prefix, unsigned max_line_length,
                           unsigned maxFirstLineLength = 0)
      : os_(&os)
      , line_prefix_(line_prefix)
      , rest_max_line_length_(max_line_length)
      , max_line_length_(max_line_length)
      , current_line_length_()
      , nlines() {
    active_instance_.reset(new FormattedOstreamIterator*(this));
    if (maxFirstLineLength)
      max_line_length_ = maxFirstLineLength;
    else
      *os_ << line_prefix;
  }

  ~FormattedOstreamIterator();

  FormattedOstreamIterator& operator=(char c);

  FormattedOstreamIterator& operator*();
  FormattedOstreamIterator& operator++();
  FormattedOstreamIterator operator++(int);


  unsigned getLine() const { return nlines; }
  /** return column not counting initial position and line_prefix. */
  unsigned getColumn() const { return current_line_length_; }

 private:
  enum { kMinLineLen = 10 };
  void newline() {
    *os_ << '\n' << line_prefix_;
    ++nlines;
    current_line_length_ = 0;
    max_line_length_ = rest_max_line_length_;
    if (max_line_length_ < kMinLineLen) max_line_length_ = kMinLineLen;  // avoid infinite loop
  }
  void insert_word();

  void write_word(std::string const& word);

  std::ostream* os_;
  std::string word_buffer_;

  std::string line_prefix_;
  unsigned rest_max_line_length_;  // first line length may be different
  unsigned max_line_length_;
  unsigned current_line_length_;
  unsigned nlines;

  shared_ptr<FormattedOstreamIterator*> active_instance_;
};


/** print hanging-indent starting from column. returns ending column. */
template <class Chars>
unsigned printFormatted(std::ostream& out, Chars const& content, unsigned startColumn,
                        std::string const& hangingIndent = "  ", unsigned endColumn = 80) {
  unsigned hangColumns = (unsigned)(endColumn-hangingIndent.size());
  FormattedOstreamIterator format(out, hangingIndent, hangColumns, endColumn-startColumn);
  for (typename Chars::const_iterator i = content.begin(), e = content.end(); i != e; ++i) format = *i;
  // format=std::copy(content.begin(), content.end(), format);
  if (format.getLine()) startColumn = (unsigned)hangingIndent.size();
  return startColumn + format.getColumn();
}

/** print a paragraph. assumes starting column is at beginning of line.  */
template <class Chars>
unsigned printFormatted(std::ostream& out, Chars const& content, std::string const& firstLineIndent = "  ",
                        std::string const& restLineIndent = "", unsigned endColumn = 80) {
  out << firstLineIndent;
  return printFormatted(out, content, (unsigned)firstLineIndent.size(), restLineIndent, endColumn);
}


}}

#endif
