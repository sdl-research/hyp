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
#include <sdl/Db/BDBConfig.hpp>
#include <sdl/Serializer/DbCxx.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/SetFlagBits.hpp>
#include <sdl/Util/TemporaryFile.hpp>
#include <sdl/Util/TinyXml.hpp>
#include <sdl/ConfigStringLiterals.hpp>

namespace sdl {

SDL_NAME_ENUM(DbaPutMode);
SDL_NAME_ENUM(BdbOpenMode);

void BDBConfig::setCreate(bool excl) {
  BDBWrapperConfig::setCreate();
  BDBEnvironmentConfig::setCreate();
  setExcl(excl);
  mode = kUD;
  validate();
}

void BDBConfig::setTemp(std::string const& tmpXXX) {
  // TODO: test
  dbName = "tempdb";
  setDbPath(Util::temporaryFile(tmpXXX, false, Util::kUnlinkTemporary));
  envPath = ".";
  overwrite = true;
  envFlags = defaultEnvFlags();
  setBdbProperty(setFlags, (sortDuplicates ? DB_DUPSORT : DB_DUP), duplicates);
  setCreate();
}

void BDBConfig::load(std::string const& filePath, bool keepDbFileAndName) {
  ticpp::Document config(filePath);
  config.LoadFile();
  ticpp::Element* pDBConfig = config.FirstChildElement(SDL_CSTR_DBCONFIG);
  load(pDBConfig, keepDbFileAndName);
}

void BDBConfig::load(ticpp::Element* pConfig, bool keepDbFileAndName) {
  std::string file, name;
  if (keepDbFileAndName) {
    std::swap(file, dbFile);
    std::swap(name, dbName);
  }
  pConfig->GetAttribute(SDL_CSTR_NAME, &dbName);
  ticpp::Element* pEnvConfig = pConfig->FirstChildElement(SDL_CSTR_BDBENVIRONMENT);
  BDBEnvironmentConfig::load(pEnvConfig);
  ticpp::Element* bdbWrapperConfig = pConfig->FirstChildElement(SDL_CSTR_BDBCONFIG);
  BDBWrapperConfig::load(bdbWrapperConfig);
  if (BDBEnvironmentConfig::encrypt) BDBWrapperConfig::setEncrypt();
  mode = kUD;
  if (keepDbFileAndName) {
    std::swap(file, dbFile);
    std::swap(name, dbName);
  }
  validate();
}

void BDBConfig::validate(bool debugLogging) {
  if (maxHeaderSize > kMaxRuleEncodingHeaderSize)
    SDL_THROW_LOG(BDBConfig.maxHeaderSize, ConfigException, "maxHeaderSize is greater than compiled maximum "
                                                                << kMaxRuleEncodingHeaderSize
                                                                << " (TODO: remove this restriction)");
  if (debugLogging) {
    SDL_DEBUG(Db.BDBConfig.validate, "before validate: " << *this);
  }
  BDBWrapperConfig::setThread(multipleThreadSafe);
  if (enabled()) BDBEnvironmentConfig::setMissingEnvPathToParent(dbFile);

  if (mode != kUD) {
    openFlags = 0;
    setFlags = 0;
    if (mode == kRW)
      // TODO: test
      openFlags = DB_CREATE;
    else if (mode == kWO)
      // TODO: test
      openFlags = DB_CREATE | DB_EXCL;
    else if (mode == kRO)
      openFlags = DB_RDONLY;
    else
      SDL_THROW_LOG(Resources.DbConfig, ConfigException, "Invalid mode specified for the DB: " << *this);
  }
  setBdbProperty(setFlags, DB_ENCRYPT, encrypt);

  BDBEnvironmentConfig::validate();
  BDBWrapperConfig::validate();

  validateDuplicates();

  if (debugLogging) {
    SDL_DEBUG(Db.BDBConfig.validate, "after validate: " << *this);
  }
}


}
