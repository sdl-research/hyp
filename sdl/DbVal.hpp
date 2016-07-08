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

 .
*/

#ifndef DBVAL_JG_2014_10_20_HPP
#define DBVAL_JG_2014_10_20_HPP
#pragma once

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#if SDL_LMDB
#include <lmdb.h>
#else
struct MDB_val {
  std::size_t mv_size;
  void* mv_data;
};
#endif

inline bool operator==(MDB_val const& a, MDB_val const& b) {
  return a.mv_data == b.mv_data && a.mv_size == b.mv_size;
}

inline bool contentsEqual(MDB_val const& a, MDB_val const& b) {
  if (!a.mv_data)
    return !b.mv_data;
  else if (!b.mv_data)
    return false;
  else
    return a.mv_size == b.mv_size && !std::memcmp(a.mv_data, b.mv_data, a.mv_size);
}

std::ostream& operator<<(std::ostream& o, MDB_val const& mv);

#define SDL_LOG_DB_CONTENTS(desc, dbval) \
  SDL_TRACE(db.contents, desc << " (" << dbval.mv_size << " bytes): " << dbval)
/// return true if truncated (can print ... or something)
bool escape3(MDB_val const& mv, std::string& s, std::size_t maxlen = 0);

inline void* data(MDB_val const& mv) {
  return mv.mv_data;
}
inline std::size_t len(MDB_val const& mv) {
  return mv.mv_size;
}
inline void*& data(MDB_val& mv) {
  return mv.mv_data;
}
inline std::size_t& len(MDB_val& mv) {
  return mv.mv_size;
}
inline unsigned char* arrayBegin(MDB_val const& mv) {
  return (unsigned char*)mv.mv_data;
}
inline unsigned char* arrayEnd(MDB_val const& mv) {
  return (unsigned char*)mv.mv_data + mv.mv_size;
}

inline void swap(MDB_val& a, MDB_val& b) {
  MDB_val tmp;
  std::memcpy(&tmp, &a, sizeof(MDB_val));
  std::memcpy(&a, &b, sizeof(MDB_val));
  std::memcpy(&b, &tmp, sizeof(MDB_val));
}

namespace sdl {

typedef unsigned char byte;
typedef byte const* const_byteptr;
typedef byte* byteptr;

inline void advance(MDB_val& mv, byteptr begin) {
  assert(begin <= (byteptr)mv.mv_data + mv.mv_size);
  mv.mv_size -= (begin - (byteptr)mv.mv_data);
  mv.mv_data = (void*)begin;
}

inline byteptr beginData(MDB_val const& mv) {
  return (byteptr)mv.mv_data;
}

inline byteptr endData(MDB_val const& mv) {
  return (byteptr)mv.mv_data + mv.mv_size;
}

typedef ::MDB_val DbVal;

inline void realloc(DbVal& buf, std::size_t sz) {
  buf.mv_data = std::realloc(buf.mv_data, buf.mv_size = sz);
}

inline void alloc(DbVal& buf, std::size_t sz) {
  buf.mv_data = std::malloc(buf.mv_size = sz);
}

inline void realloc(DbVal& buf) {
  buf.mv_data = std::realloc(buf.mv_data, buf.mv_size);
}

inline void reallocAtLeast(DbVal& buf, std::size_t sz) {
  if (sz > buf.mv_size) buf.mv_data = std::realloc(buf.mv_data, buf.mv_size = sz);
}


/// like auto_ptr-assignment from releases ownership
struct MallocedDbVal : DbVal {
  std::size_t alloced_size;
  // since mv_size (used size) may be less, this makes it safer to reuse the
  // allocation later


  MallocedDbVal() { mv_data = 0; }

  MallocedDbVal(std::size_t sz) { mv_data = (alloced_size = mv_size = sz) ? std::malloc(sz) : 0; }

  MallocedDbVal(MallocedDbVal const& o) { operator=(o); }

  void reuse() { mv_size = alloced_size; }

  void operator=(MallocedDbVal const& o) {
    static_cast<DbVal&>(*this) = o;
    o.disown();
  }

  void realloc(std::size_t sz) {
    assert(sz);
    mv_data = std::realloc(mv_data, alloced_size = mv_size = sz);
  }

  DbVal release() {
    DbVal r(*this);
    disown();
    return r;
  }

  ~MallocedDbVal() {
    if (mv_data) std::free(mv_data);
  }

 private:
  void disown() const { const_cast<void*&>(mv_data) = 0; }
};

inline void setDbVal(DbVal& r, std::size_t sz, void* data) {
  r.mv_size = sz;
  r.mv_data = data;
}

template <class Dbt>
inline void setDbVal(DbVal& r, Dbt const& dbt) {
  r.mv_size = dbt.get_size();
  r.mv_data = dbt.get_data();
}

inline DbVal dbVal(std::size_t sz, void* data) {
  DbVal r;
  r.mv_size = sz;
  r.mv_data = data;
  return r;
}

template <class Dbt>
inline DbVal dbValFrom(Dbt const& dbt) {
  DbVal r;
  r.mv_size = dbt.get_size();
  r.mv_data = dbt.get_data();
  return r;
}

/// fine for bdb put
template <class Dbt>
inline void dbValForPut(DbVal v, Dbt& dbt) {
  dbt.set_size((unsigned)v.mv_size);
  dbt.set_data((void*)v.mv_data);
}

struct DbException : std::exception {
  char const* what_;
  DbException(char const* kCstr = "Db error") : what_(kCstr) {}
  DbException(DbException const& o) : what_(o.what_) {}
  ~DbException() throw() {}
  char const* what() const throw() { return what_; }
};

struct MdbException : DbException {
  MdbException(char const* kCstr = "Mdb error") : DbException(kCstr) {}
  ~MdbException() throw() {}
};

struct OpenDbException : DbException {
  OpenDbException(char const* err = "Couldn't open Db") : DbException(err) {}
};

struct KeyNotFoundException : DbException {
  KeyNotFoundException(char const* err = "key not found in db") : DbException(err) {}
};

struct DbBufferTooSmallException : DbException {
  DbBufferTooSmallException(char const* err = "db buffer too small") : DbException(err) {}
};

DbBufferTooSmallException const kDbBufferTooSmall("Db output buffer too small");

inline void checkBufferSize(const_byteptr out, const_byteptr outEnd, std::size_t sz) {
  if (out + sz > outEnd) throw kDbBufferTooSmall;
}


}

#endif
