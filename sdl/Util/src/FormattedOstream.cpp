#include <cctype>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>





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


    {

      write_word(word_buffer_.substr(i, max_line_length_));
    }
  }
  word_buffer_ = "";
}

void FormattedOstreamIterator::write_word(const std::string& word) {
  *os_ << word;

  if (current_line_length_ != max_line_length_) {
    *os_ << ' ';
    ++current_line_length_;
  }
}


