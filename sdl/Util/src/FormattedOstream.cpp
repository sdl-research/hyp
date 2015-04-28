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
#include <cctype>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <sdl/SharedPtr.hpp>

#include <sdl/Util/FormattedOstream.hpp>

namespace sdl {
namespace Util {

FormattedOstreamIterator::~FormattedOstreamIterator() {
  if (*active_instance_ == this)
    insert_word();
}

FormattedOstreamIterator& FormattedOstreamIterator::operator = (char c) {
  *active_instance_ = this;
  if (std::isspace(c)) {
    if (word_buffer_.size() > 0) {
      insert_word();
    }
  }
  else {
    word_buffer_.push_back(c);
  }
  return *this;
}

FormattedOstreamIterator& FormattedOstreamIterator::operator*()     { return *this; }
FormattedOstreamIterator& FormattedOstreamIterator::operator++()    { return *this; }
FormattedOstreamIterator FormattedOstreamIterator::operator++(int) { return *this; }

void FormattedOstreamIterator::insert_word() {
  if (word_buffer_.size() == 0)
    return;

  if (word_buffer_.size() + current_line_length_ <= max_line_length_) {
    write_word(word_buffer_);
  } else {
    for (unsigned i = 0, e = (unsigned)word_buffer_.size(); i<e; i += max_line_length_)
    {
      newline();
      write_word(word_buffer_.substr(i, max_line_length_));
    }
  }
  word_buffer_ = "";
}

void FormattedOstreamIterator::write_word(const std::string& word) {
  *os_ << word;
  current_line_length_ += (unsigned)word.size();
  if (current_line_length_ != max_line_length_) {
    *os_ << ' ';
    ++current_line_length_;
  }
}

}}
