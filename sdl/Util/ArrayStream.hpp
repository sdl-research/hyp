/** \file

    fixed size user-supplied memory read/write stream buffer
*/

#ifndef ARRAYSTREAM_JG20121120_HPP
#define ARRAYSTREAM_JG20121120_HPP
#pragma once

#include <graehl/shared/array_stream.hpp>
#include <sdl/SharedPtr.hpp>

namespace sdl { namespace Util {

typedef graehl::array_streambuf ArrayStreambuf;
typedef graehl::array_stream ArrayStream;

struct ArrayStreambufHolder
{
  ArrayStreambufHolder(std::size_t buflen)
      : capacity_(buflen)
      , buf_((char*)malloc(buflen))
      , sbuf_(buf_, buflen)
  {}

  /// return # of bytes written
  std::size_t size() const { return sbuf_.size(); }

  char *data() const { return buf_; }

  std::streambuf &streambuf() {
    return sbuf_;
  }

  std::streambuf &streambufResetWrite() {
    sbuf_.reset_write();
    assert(sbuf_.size() == 0);
    return sbuf_;
  }

  ~ArrayStreambufHolder() { std::free(buf_); }
 protected:
  std::size_t capacity_;
  char *buf_;
  Util::ArrayStreambuf sbuf_;
};

struct StringArrayStream : ArrayStream {
  typedef shared_ptr<std::string> StringPtr;
  StringPtr str_;
  StringArrayStream() : str_() {}
  StringArrayStream(std::string *str) : str_(str) { syncFromStr(); }
  StringArrayStream(StringPtr const& str) : str_(str) { syncFromStr(); }

  void setRef(std::string &string) {
    setNoDelete(str_, string);
    syncFromStr();
  }
  void set(StringPtr const& str) {
    str_ = str;
    syncFromStr();
  }
  void setNew(std::string *str)
  {
    str_.reset(str);
    syncFromStr();
  }
  /**
     take contents of str_ and init stream to read pointer and write pointer both to begin
  */
  void syncFromStr() {
    clear(); // clear eof etc
    if (str_)
      set_array(*str_);
    else
      set_array();
  }

  void reset() {
    str_.reset();
    syncFromStr();
  }

};

}}

#endif
