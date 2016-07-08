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
/*
  BDBEnvironmentConfig.
*/

#ifndef SDL_BDBENVIRONMENTCONFIG_HPP
#define SDL_BDBENVIRONMENTCONFIG_HPP
#pragma once

#include <sdl/Config/Init.hpp>
#include <sdl/Db/BdbFlags.hpp>
#include <sdl/Util/Ticpp-fwd.hpp>
#include <sdl/BDB-fwd.hpp>
#include <sdl/Path.hpp>
#include <boost/functional/hash.hpp>
#include <graehl/shared/hex_int.hpp>
#include <iostream>
#include <string>

namespace sdl {

/**
   \class BDBEnvironmentConfig
   Configuration for BDB Environment

   \see BDBEnvironment and BDBConfig
*/
struct BDBEnvironmentConfig {
 public:
  //  void load(std::string const&);
  void load(ticpp::Element*);
  //  void parse(std::string const&);

  void setThread(bool thread = true);

  void setCreate(bool create = true);

  BDBEnvironmentConfig() { Config::inits(this); }

  template <class Config>
  void configure(Config& config) {
    config.is("BDB environment config");
    config("Berkeley DB environment config");
    config("cache", &cacheInMB)("Cache size in MB").init(200);
    config("env-path", &envPath)("db environment dir (if empty, use dir holding .db file)").init(".");
    config("encrypt", &encrypt)(
        "Encrypt? (setting should match whenever writing to a given .db file; for reading, we always try to "
        "decrypt)")
        .init(false);
    config("free-threaded", &multipleThreadSafe)
        .init(true)(
            "Initialize the environment for multiple threads (we'd like to share the environment "
            "process-wide even if open Dbs are not)");
    config("env-flags", &envFlags)("BDB Environment flags").init(defaultEnvFlags());
  }

  static BdbEnvFlags defaultEnvFlags();

  std::string envPath;
  BdbEnvFlags envFlags;
  std::size_t cacheInMB;
  bool encrypt;
  bool multipleThreadSafe;

  friend inline std::ostream& operator<<(std::ostream& out, BDBEnvironmentConfig const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const {
    out << "env-path: " << envPath;
    out << " env-flags: " << envFlags;
    out << " encrypt: " << encrypt;
  }

  void setMissingEnvPathToParent(boost::filesystem::path const& file) {
    if (envPath.empty()) setEnvPathToParent(file);
  }

  void setEnvPathToParent(boost::filesystem::path const& file) {
    if (file.is_absolute())
      setEnvPath(file.parent_path());
    else
      envPath = ".";
  }

  void setEnvPath(boost::filesystem::path const& path) { envPath = path.string(); }

  explicit BDBEnvironmentConfig(std::string const& envPath, BdbEnvFlags envFlags = defaultEnvFlags(),
                                std::size_t cacheInMB = 100, bool encrypt = false, bool multipleThreadSafe = true)
      : envPath(envPath)
      , envFlags(envFlags)
      , cacheInMB(cacheInMB)
      , encrypt(encrypt)
      , multipleThreadSafe(multipleThreadSafe) {
    if (multipleThreadSafe) setThread();
  }
  friend inline void validate(BDBEnvironmentConfig& x) { x.validate(); }
  void validate() {
    setThread(multipleThreadSafe);
    validateBdbFlags(envFlags);
  }

  bool operator==(BDBEnvironmentConfig const& o) const { return envPath == o.envPath; }

  std::size_t hash_value() const {
    return boost::hash<std::string>()(envPath);
    // TODO: detect attempts to use different flags or incorporate flags into hash
  }

 protected:
  void doParse(ticpp::Element*);
};


}

#endif
