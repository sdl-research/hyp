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

    type-safe user-friendly bit fields for Berkeley DB.
*/

#ifndef BDBFLAGS_JG_2014_09_26_HPP
#define BDBFLAGS_JG_2014_09_26_HPP
#pragma once

#include <sdl/Serializer/Db.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/SetFlagBits.hpp>
#include <graehl/shared/configure_named_bits.hpp>

namespace sdl {

typedef unsigned BdbFlagsInt;

inline void setBdbProperty(BdbFlagsInt& flags, BdbFlagsInt property, bool val) {
  Util::setFlagBits<BdbFlagsInt>(flags, property, val);
}

#define SDL_BDB_BITNAME(x) bits(#x, x)

struct BdbOpenFlagNames {
  template <class Bits>
  static void bits(Bits& bits) {
    SDL_BDB_BITNAME(DB_CREATE);  // 1
    /**
       EXCL means fail if file already exists; allowed only if DB_CREATE.
    */
    SDL_BDB_BITNAME(DB_EXCL);  // 4
    SDL_BDB_BITNAME(DB_RDONLY);  // x400
    SDL_BDB_BITNAME(DB_THREAD);  // x20
    SDL_BDB_BITNAME(DB_TRUNCATE);  // 0x20000
    // TODO: many more if we want
  }
};

struct BdbSetFlagNames {
  template <class Bits>
  static void bits(Bits& bits) {
    SDL_BDB_BITNAME(DB_DUP);  // 0x10
    SDL_BDB_BITNAME(DB_DUPSORT);  // 0x2
    SDL_BDB_BITNAME(DB_ENCRYPT);  // 0x1
    // TODO: many more if we want
  }
};

struct BdbEnvFlagNames {
  template <class Bits>
  static void bits(Bits& bits) {
    SDL_BDB_BITNAME(DB_PRIVATE);  // 0x10000
    SDL_BDB_BITNAME(DB_INIT_MPOOL);  // 0x400
    SDL_BDB_BITNAME(DB_CREATE);  // 1
    SDL_BDB_BITNAME(DB_THREAD);  // 0x20
    assert(DB_THREAD == 0x20);
    // TODO: many more if we want
  }
};

struct BdbPutFlagNames {
  template <class Bits>
  static void bits(Bits& bits) {
    SDL_BDB_BITNAME(DB_MULTIPLE_KEY);  // x4000
    SDL_BDB_BITNAME(DB_MULTIPLE);  // x800

    bits.overlapping();
    // &0xFF are actually enumerations so you'll get nonsense if >1 is set
    SDL_BDB_BITNAME(DB_NODUPDATA);  // 19-for dupsort dbs, don't insert a duplicate (key, data)
    SDL_BDB_BITNAME(DB_OVERWRITE_DUP);  // 21-for bulk put, silently continue on existing key (adding the
    // new data) - must be used instead of DB_NODUPDATA for DB_MULTIPLE*
    SDL_BDB_BITNAME(DB_NOOVERWRITE);  // 20-for one-at-a-time put, return status code on existing key
    // TODO: there are more
  }
};

/// to set a different mode, clear these bits first
BdbFlagsInt const kBdbPutFlagModeMask = 0xFF;

inline BdbFlagsInt bdbPutMode(BdbFlagsInt flags) {
  return flags & kBdbPutFlagModeMask;
}

inline void setBdbPutMode(BdbFlagsInt& flags, BdbFlagsInt mode) {
  assert((mode & kBdbPutFlagModeMask) == mode);
  flags &= ~(BdbFlagsInt)kBdbPutFlagModeMask;
  flags |= mode;
}

/// or multiplePut = DB_MULTIPLE
inline BdbFlagsInt bdbPutFlagsMultiple(BdbFlagsInt putMode, BdbFlagsInt multiplePut = DB_MULTIPLE_KEY) {
  putMode &= kBdbPutFlagModeMask;
  return (putMode == DB_NODUPDATA || putMode == DB_OVERWRITE_DUP) ? multiplePut | DB_OVERWRITE_DUP : multiplePut;
}

struct BdbGetFlagNames {
  template <class Bits>
  static void bits(Bits& bits) {
    SDL_BDB_BITNAME(DB_MULTIPLE);
    SDL_BDB_BITNAME(DB_CONSUME);  // for queue
    SDL_BDB_BITNAME(DB_CONSUME_WAIT);  // for queue
    SDL_BDB_BITNAME(DB_SET_RECNO);  // for DB_RECNUM opened/created BTree dbs
    // TODO: there are more
  }
};

#undef SDL_BDB_BITNAME

using graehl::named_bits;
typedef named_bits<BdbOpenFlagNames, BdbFlagsInt> BdbOpenFlags;
typedef named_bits<BdbSetFlagNames, BdbFlagsInt> BdbSetFlags;
typedef named_bits<BdbEnvFlagNames, BdbFlagsInt> BdbEnvFlags;
typedef named_bits<BdbPutFlagNames, BdbFlagsInt> BdbPutFlags;
typedef named_bits<BdbGetFlagNames, BdbFlagsInt> BdbGetFlags;

template <class Flags>
void validateBdbFlags(Flags& flags) {
  BdbFlagsInt unk = flags.clear_unknown();
  if (unk)
    SDL_WARN(Db.BdbFlags, "cleared unknown BDB flags: " << graehl::hex(unk)
                                                        << " - allowed: " << flags.usage(true));
}


}

#endif
