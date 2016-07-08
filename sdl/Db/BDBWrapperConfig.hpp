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
    BDB database location/name/options
*/

#ifndef SDL_BDBWRAPPERCONFIG_HPP
#define SDL_BDBWRAPPERCONFIG_HPP
#pragma once

#include <sdl/Db/BdbFlags.hpp>
#include <sdl/Serializer/BDBDefaults.hpp>
#include <sdl/Util/Enum.hpp>
#include <sdl/Util/Ticpp-fwd.hpp>
#include <sdl/BDB-fwd.hpp>
#include <sdl/Dba.hpp>

namespace sdl {

SDL_ENUM(BdbIndexType, 5, (BTree, Hash, Heap, Queue, RecNo))

/**
   Configuration for a BDB Database

   \detail This class implements a parser to read initial BDB DB
   configuration from xml files.

   \see BDBWrapper
*/
struct BDBWrapperConfig : SubDbConfig {
  BDBWrapperConfig() : overwrite(), pageSize(), indexType(kBTree), throwOnNoKey(false) {
    defaultBufferSize();
    maxAttempts = 1;
  }

  template <class Config>
  void configure(Config& config) {
    SubDbConfig::configure(config);
    config.is("BDB database");
    config("index", &indexType).init(kBTree)("Index type (BTree|Hash|Heap|Queue|RecNo)");
    config("buffer", &bufferSize)
        .init(kDefaultBufferSize)(
            "Buffer Size in bytes (for holding a batch of multiple values). Defaults to 8192 Bytes (8 KB)");
    config("max-buffer", &maxBufferSize)
        .init(kDefaultMaxBufferSize)(
            "Maximum buffer size for reading records (only grows to this large if needed)");
    config("page-size", &pageSize).init(0)("if nonzero, override default page size (in bytes)");
    config("db-flags", &openFlags)
        .init(0)(
            "(setting must match across creation and reading of DB) Db::open() flags e.g. "
            "DB_RDONLY|DB_THREAD");
    config("set-flags", &setFlags)
        .init(0)(
            "(setting must match across creation and reading of DB) Db::set_flags() flags e.g. "
            "DB_DUPSORT|DB_ENCRYPT");
    config("max-attempts", &maxAttempts)
        .init(1)(
            "Number of tries (e.g. recover from intermittent remote filesystem unavailability) before giving "
            "up a db read/write");
    config("file-path",
           &dbFile)("berkeley.db filename containing one or more databases (select one with 'name')");
  }

  /// to be called before creating bdb object
  void setAllowDuplicates(bool allow = true, bool sort = true) {
    sortDuplicates = sort;
    duplicates = allow;
    validateDuplicates();
  }

  /// fix flags related to DB_DUP/DB_DUPSORT based on 'duplicates' and
  /// 'sortDuplicates' settings (if validate was already called, you can just
  /// call this after adjusting that)
  void validateDuplicates();

  bool hasDuplicates() const;

  bool enabled() const { return !dbFile.empty(); }

  void load(ticpp::Element*);

  void defaultBufferSize() {
    bufferSize = kDefaultMaxBufferSize;
    maxBufferSize = kDefaultMaxBufferSize;
  }


  BDBWrapperConfig(std::string const& subdbname, std::string const& dbFile,
                   BdbOpenFlags openFlags,  // e.g. DB_RDONLY | DB_THREAD
                   BdbSetFlags setFlags,  // e.g. DB_DUPSORT | DB_ENCRYPT
                   unsigned maxAttempts, BdbIndexType indexType = kHash, unsigned pageSize = 0)
      : dbFile(dbFile)
      , openFlags(openFlags)
      , setFlags(setFlags)
      , maxAttempts(maxAttempts)
      , indexType(indexType)
      , pageSize(pageSize)
      , overwrite() {
    this->dbName = subdbname;
    defaultBufferSize();
  }

  // path (could be relative to BDBEnvironment envPath) to .db file
  std::string dbFile;
  BdbOpenFlags openFlags;
  BdbSetFlags setFlags;
  unsigned maxAttempts;
  BdbIndexType indexType;
  unsigned pageSize;

  bool indexTypeAllowsDuplicates() const { return indexType == kBTree || indexType == kHash; }

  void setThread(bool thread = true);

  void setEncrypt(bool encrypt = true);

  void setCreate(bool create = true);

  /// excl: must not already exist (implies create)
  void setExcl(bool excl = true);

  bool creating() const;

  void setWrite() { setCreate(); }

  void setReadOnly();

  /// remove old db when creating - see BDBFactory::getDB
  bool overwrite = false;

  unsigned bufferSize;  // starts small and increases (*=5) until reaching max
  unsigned maxBufferSize;

  friend inline std::ostream& operator<<(std::ostream& out, BDBWrapperConfig const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const {
    out << "name: '" << dbName << "' type: " << indexType << " file: '" << dbFile << "' open-flags: " << openFlags;
    if (setFlags) out << " set-flags: " << setFlags;
  }

  friend inline void validate(BDBWrapperConfig& x) { x.validate(); }
  void validate() {
    if (maxAttempts < 1) maxAttempts = 1;
    validateBdbFlags(openFlags);
    validateBdbFlags(setFlags);
  }

  void operator=(SubDbConfig const& sub) {
    dbName = sub.dbName;
    setAllowDuplicates(sub.duplicates, sub.sortDuplicates);
  }

 protected:
  bool throwOnNoKey;
};


}

#endif
