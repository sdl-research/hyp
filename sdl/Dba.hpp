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

    berkeley db / Lightning mdb read abstraction layer

    Dba (env) (per process shared is ok) -> txn(must be per-thread) -> db(must be per-thread).

*/

#ifndef DB_FWD_JG_2014_09_02_HPP
#define DB_FWD_JG_2014_09_02_HPP
#pragma once

#include <sdl/Util/Delete.hpp>
#include <sdl/Util/Enum.hpp>
#include <sdl/DbVal.hpp>
#include <sdl/SharedPtr.hpp>
#include <boost/functional/hash.hpp>
#include <stdexcept>
#include <stddef.h>

namespace sdl {

enum { kDbOpenUniqueKeys = false, kDbOpenAllowDuplicateKeys = true };
enum { kDbDuplicatesUnsorted = false, kDbDuplicatesSorted = true };

struct DBWrapper;
struct BDBWrapper;
struct MdbEnv;

/**
   TODO: fn pointer + data (or mem fn pointer + this) are both faster than interface w/ single virtual

*/
struct AcceptDbKeyData {
  /// override this optionally
  virtual void operator()(DbVal data) {
    SDL_THROW_LOG(AcceptDbKeyData.data, UnimplementedException,
                  "undefined - probably you need to call (key, data) and not just (data)");
  }
  /// or else override this (or override both)
  virtual void operator()(DbVal, DbVal data) { operator()(data); }
};

struct SubDbConfig {
  std::string dbName;
  bool duplicates;
  bool sortDuplicates;
  bool reversedKeys;
  bool readOnly;
  bool create;
  bool overwrite;
  bool operator==(SubDbConfig const& o) const {
    return dbName == o.dbName && duplicates == o.duplicates && sortDuplicates == o.sortDuplicates;
  }
  SubDbConfig(bool duplicates = true, bool sortDuplicates = true)
      : duplicates(duplicates)
      , sortDuplicates(sortDuplicates)
      , reversedKeys(true)
      , readOnly(false)
      , create(false)
      , overwrite(false) {}
  template <class Config>
  void configure(Config& config) {
    config("name", &dbName)(
        "Name of database (inside environment file-path). May be empty only if there are no sub-dbs (the "
        "list of dbs would be in the empty/toplevel one)");
    config("read-only", &readOnly).init(false)("don't allow writes - might be faster");
    config("create", &create)
        .init(false)("create if not already existing (or always, if create + overwrite)");
    config("overwrite", &overwrite).init(false)("if create, remove old db if one already existed");
    config("has-duplicates", &duplicates)
        .defaulted()(
            "db 'name' is to have multiple (key, data) with the same key - this setting must stay constant "
            "from one use of the db to the next. Often xmt knows which type is appropriate and ignores this "
            "option.");
    config("sort-duplicates", &sortDuplicates)
        .defaulted()(
            "(if has-duplicates) lexicographically sort the data of same-key (key, data) pairs. true might "
            "prevent duplicate (key, data) pairs. false means keep order of insertion");
    config("reversed-keys", &reversedKeys)
        .verbose()
        .init(true)(
            "sort on reversed key strings (should be more efficient for old xmt grammar dbs since an "
            "uninformative length field is at the beginning of the string)");
  }
  bool sortedDuplicates() const { return duplicates && sortDuplicates; }
  std::size_t hash_value() const { return boost::hash<std::string>()(dbName); }
};

SDL_ENUM(DbaPutMode, 3, (PutNoDupData, PutAlways, PutNoOverwrite))

struct DbaPutOptions {
  DbaPutMode mode;
  template <class Config>
  void configure(Config& config) {
    config.is("DbaPutOptions");
    config("mode", &mode).init(kPutAlways)("(no-duplicates is allowed only for a 'sort-duplicates' db)");
  }
  DbaPutOptions(DbaPutMode mode = kPutAlways) : mode(mode) {}
  operator DbaPutMode() const { return mode; }
};

/// a key->values db with whatever data buffers+logic is needed to visit each
/// matching record's bytes 1 at a time. should be used thread-exclusive in case
/// there's a local member buffer needed
struct Dba {
  /// return NULL if key not found, else new object decoded from key, data pair
  /// (see Serializer/KeyDataCodec for e.g. BoostKeyDataCodec)
  template <class KeyDataDecode>
  typename KeyDataDecode::value_type* newSingleData(KeyDataDecode const& decode, DbVal const& key) {
    typedef typename KeyDataDecode::value_type Value;
    DbVal data((this->singleData(key)));
    if (data.mv_data) {
      Util::AutoDelete<Value> r(new Value);
      decode.decodeKey(*r, key);
      decode.decodeData(*r, data);
      return r.release();
    } else
      return NULL;
  }

  /// given key, call accept(data) for every data such that (key, data) is in db
  virtual void data(DbVal const& key, AcceptDbKeyData& accept) = 0;

  /// same as data(key, accept) except we call accept.accept(key, data) for given key
  virtual void keyData(DbVal const& key, AcceptDbKeyData& accept) = 0;

  /// \return true if data(key, accept) will accept anything. but subclass
  /// should override for efficiency
  virtual bool contains(DbVal const& key) = 0;

  /// returns the single data for key else NULL data if no match (so 0-length
  /// but non-NULL means a matching (key, empty data) pair
  virtual DbVal singleData(DbVal const& key) = 0;

  /// as singleData but throws DbException if no value
  DbVal singleMandatoryData(DbVal const& key) {
    DbVal r((singleData(key)));
    if (!r.mv_data) throw DbException("missing mandatory data for key");
    return r;
  }

  /// call accept(key,data) for every (key, data) pair in db
  virtual void every(AcceptDbKeyData& accept) = 0;

  // no virtual dtor needed if you properly make_shared<DbImpl>(...), but for safety, keep it
  virtual ~Dba() {}

  // TODO: put write operations directly in Dba + rename to Db
  virtual DBWrapper* getDb() { return 0; };

  friend inline std::ostream& operator<<(std::ostream& out, Dba const& self) {
    self.print(out);
    return out;
  }
  virtual void print(std::ostream& out) const { out << name(); }

  /// for sort-duplicates dbs only, avoid repeating existing (key, data)
  virtual void putNoDupData(DbVal const& key, DbVal const& data) { put(key, data, kPutNoDupData); }

  /// return false iff key already existed
  virtual bool putNoOverwrite(DbVal const& key, DbVal const& data) { return put(key, data, kPutNoOverwrite); }

  /// adds (key, data) without complaint if key already existed
  virtual void put(DbVal const& key, DbVal const& data) { put(key, data, kPutAlways); }

  /// return false iff mode nooverwrite and key already existed
  virtual bool put(DbVal const& key, DbVal const& data, DbaPutOptions mode) {
    throw DbException("putNoDupData(key, data) not implemented");
  }

  Dba() {}
  explicit Dba(SubDbConfig const& config) : config_(config) {}
  std::string const& name() const { return config_.dbName; }
  SubDbConfig config_;
};

struct AcceptDbVal {
  virtual void operator()(DbVal data) = 0;
};

struct PutDataForKey : AcceptDbVal, DbaPutOptions {
  Dba& db;
  DbVal key;
  PutDataForKey(Dba& db, DbVal const& key, DbaPutOptions putting)
      : DbaPutOptions(putting), db(db), key(key) {}
  void operator()(DbVal data) override { db.put(key, data, *this); }
};

typedef shared_ptr<Dba> DbaPtr;

/// named sub-databases factory. return DbaPtr must stay valid even if Dbas is destroyed.
struct Dbas {
  /**
     may throw OpenDbException. if allowDuplicateKeys == kDbOpenUniqueKeys then you may only retrieve a single
     data for a given key
  */
  DbaPtr openSubDup(std::string const& subdb = std::string(),
                    bool allowDuplicateKeys = kDbOpenAllowDuplicateKeys, bool create = false) {
    SubDbConfig conf;
    conf.dbName = subdb;
    conf.create = create;
    conf.readOnly = !create;
    conf.duplicates = allowDuplicateKeys;
    return open(conf);
  }

  DbaPtr open(std::string const& subdb = std::string(), bool allowDuplicateKeys = kDbOpenAllowDuplicateKeys,
              bool create = false) {
    return openSubDup(subdb, allowDuplicateKeys, create);
  }

  DbaPtr openUniqueKeys(std::string const& subdb = std::string(), bool create = false) {
    return openSubDup(subdb, kDbOpenUniqueKeys, create);
  }

  DbaPtr openUnnamed(bool allowDuplicateKeys = kDbOpenAllowDuplicateKeys) {
    return open(std::string(), allowDuplicateKeys);
  }

  virtual DbaPtr open(SubDbConfig const& subconf) = 0;

  virtual bool exists(std::string const& subdb = std::string()) {
    try {
      (void)open(subdb, false);
      return true;
    } catch (OpenDbException&) {
      return false;
    }
  }

  virtual ~Dbas() {}

  virtual std::string name() const = 0;

  friend inline std::ostream& operator<<(std::ostream& out, Dbas const& self) {
    self.print(out);
    return out;
  }

  virtual void print(std::ostream& out) const { out << name(); }
};

typedef shared_ptr<Dbas> DbasPtr;


}

#endif
