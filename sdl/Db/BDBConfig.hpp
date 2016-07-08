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

    how to read from or create a berkeley DB.

    TODO: replace by variant Db/DbOptions.hpp which supports this and other
    databases

*/

#ifndef SDL_RESOURCES_DBCONFIG_HPP
#define SDL_RESOURCES_DBCONFIG_HPP
#pragma once

#include <sdl/Config/Init.hpp>
#include <sdl/Db/BDBWrapperConfig.hpp>
#include <sdl/Serializer/BDBEnvironmentConfig.hpp>
#include <sdl/Util/Enum.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/ProgramOptions.hpp>
#include <stdexcept>
#include <string>

namespace sdl {

// TODO: BDBConfig::maxHeaderSize ?
enum { kMaxRuleEncodingHeaderSize = 256 * 1024 * 1024 };

/// DB Mode [RO (read only)| RW (read/write) | WO (write only) | UD (User Defined-rely on numeric bdb flags
/// config)
SDL_ENUM(BdbOpenMode, 4, (RO, RW, WO, UD));

struct BDBConfig : BDBEnvironmentConfig, BDBWrapperConfig {
  void operator=(SubDbConfig const& o) { BDBWrapperConfig::operator=(o); }

  BDBConfig() {
    Config::inits(this);
    validate(false);
  }

  friend inline std::ostream& operator<<(std::ostream& out, BDBConfig const& self) {
    self.print(out);
    return out;
  }

  void print(std::ostream& out) const {
    BDBEnvironmentConfig::print(out);
    out << ' ';
    BDBWrapperConfig::print(out);
  }

  template <class Config>
  void configure(Config& config) {
    BDBEnvironmentConfig::configure(config);
    BDBWrapperConfig::configure(config);
    config("Berkeley DB config.");
    config.is("BDB config");
    config("mode", &mode)
        .init(kRO)(
            "DB Mode [RO (read only)| RW (read/write) | WO (write only - meaning db can't previously exist - "
            "so use 'overwrite' option) | UD (User Defined, Advanced mode for manually specifying flags - "
            "flags will be modified by other options unless UD is selected.)]");
    config("max-header-size", &maxHeaderSize)
        .init(100 * 1024
              * 1024)("maximum header size in bytes for databases that have a header (e.g. rules)");
  }

  friend inline void validate(BDBConfig& x) { x.validate(); }

  void init() { validate(); }

  /// set dbFile and also set environment dir to `dirname file`
  void setDbPath(std::string const& file) {
    dbFile = file;
    BDBEnvironmentConfig::setEnvPathToParent(file);
  }

  void setCreate(bool excl = true);

  /// also sets create, of course
  void setTemp(std::string const& tmpXXX = "/tmp/tmp.xmt.db.XXXXXXXX");

  void validate(bool debugLogging = true);

  std::size_t maxHeaderSize;
  BdbOpenMode mode;

  /// if keepDbFileAndName, we don't load any "name" attribute from the db xml
  /// file, which really should be deprecated by now
  void load(std::string const& filename, bool keepDbFileAndName = false);
  void load(ticpp::Element* xml, bool keepDbFileAndName = false);
};

/// return null if you should just use base, else a new copy (that you must
/// delete) - put it in a Util::ExistingOrTemporary!
inline BDBConfig* ptrSubBdbConfig(BDBConfig const& base, SubDbConfig const& sub) {
  if (sub == base)
    return 0;
  else {
    BDBConfig* override = new BDBConfig(base);
    *override = sub;
    override->validateDuplicates();
    return override;
  }
}

template <class Opt>
inline void dbopt(Opt& opt, BDBConfig& dbconf, bool nameAndFile, std::string* xmlfile) {
  using boost::program_options::value;
  opt("db-type,D", value(&dbconf.indexType)->default_value(kBTree),
      "(if you don't use db-config) set the db index type btree or hash");
  opt("db-encrypt,e", value(&dbconf.encrypt)->default_value(false), "write an encrypted db");
  opt("db-cache,c", value(&dbconf.cacheInMB)->default_value(100), "DB cache in MB");
  if (nameAndFile) {
    opt("db-name,n", value(&dbconf.dbName), "db name.");
    opt("db-file,f", value(&dbconf.dbFile), "db environment file name.");
  }
  if (xmlfile)
    opt("db-config,d", value(xmlfile), "optional xml config (overrides db-* options except name and file)");
}


}

#endif
