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

    Vocabulary configuration

    See xmt/VocabularyResource.hpp to
    instantiate a vocabulary from configuration.

    TODO@SK: move that resource to Vocabulary dir
*/

#ifndef SDL_VOCABULARY_VOCABULARYCONFIG_HPP
#define SDL_VOCABULARY_VOCABULARYCONFIG_HPP
#pragma once

#include <sdl/Config/Validate.hpp>
#include <sdl/Db/BDBConfig.hpp>
#include <sdl/Util/Enum.hpp>
#include <sdl/Sym.hpp>

namespace sdl {

struct LegacyVocabularyConfig {
  template <class Config>
  void configure(Config& config) {
    config("Legacy (read-only) vocabulary");
    config("source-vocab-path", &sourceVocabPath_)("Path to the file containing list of all source tokens.");
    config("target-vocab-path", &targetVocabPath_)("Path to the file containing list of all source tokens.");
  }

  std::string sourceVocabPath_;
  std::string targetVocabPath_;

  friend inline std::ostream& operator<<(std::ostream& out, LegacyVocabularyConfig const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const {
    out << "source-vocab: " << sourceVocabPath_;
    out << " target-vocab: " << targetVocabPath_;
  }
};

/**
   this struct is only for sucking up old config values and ignoring them
*/
struct ResidentVocabularyConfig {

  /**
     this struct is only for sucking up old config values and ignoring them
  */
  struct DummyBasicVocabularyConfig {
    DummyBasicVocabularyConfig() : dummyId() {}

    template <class Config>
    void configure(Config& config) {
      config("Basic vocabulary");
      config(
          "Basic Vocabulary declaration (note: the values you configure will be ignored; this exists only "
          "for backward config compatability.");
      config("max", &dummyId)("Max ID this symbol can take.").defaulted().deprecate();
      config("min", &dummyId)("Min ID this symbol can have.").defaulted().deprecate();
      config("next", &dummyId)("Next available ID.").defaulted().deprecate();
      // FIXME: why is this configurable at all instead of always =min?
    }

   private:
    unsigned dummyId;
  };

  ResidentVocabularyConfig() {}

  template <class Config>
  void configure(Config& config) {
    config(
        "Vocabulary which is empty to begin with and keeps all symbols in memory (note: the values you "
        "configure will be ignored; this exists only for backward config compatability.");
    config("Resident vocabulary");
    config("terminal", &vocabularies_[0])("Vocabulary range for terminal symbols.").deprecate();
    config("non-terminal", &vocabularies_[1])("Vocabulary range for non-terminal symbols.").deprecate();
    config("variable", &vocabularies_[2])("Vocabulary range for variable symbols.").deprecate();
    // FIXME: we should specify # of symbols desired, or split points, instead of redundantly specifying
    // min[1]=max[0]+1, etc.
  }

  DummyBasicVocabularyConfig vocabularies_[3];
};

struct NonResidentVocabularyConfig : BDBConfig {
  std::string mdbenv;
  bool isMdb() const { return !mdbenv.empty(); }

  template <class Config>
  void configure(Config& config) {
    config("Disk (Berkeley DB) backed vocabulary");
    config("mdbenv",
           &mdbenv)("name of mdbenv resource. use this as an alternative to all other *db* options");
    config("db-config", static_cast<BDBConfig*>(this))("DB for symbols to ID mapping.");
    config("id-to-sym-db", static_cast<BDBConfig*>(this))("for old-config portability")
        .deprecate("use db-config instead of id-to-sym-db")
        .verbose();
  }
};

inline void validate(NonResidentVocabularyConfig& x) {
  ::adl::adl_validate(static_cast<BDBConfig&>(x));
}

// we have underscores in constant names kNon_Resident so that user configuration may parse "non-resident"
// "read-only"
SDL_ENUM(VocabularyFormat, 4, (Legacy, Resident, Non_Resident, Read_Only));

/**
   Vocabulary configuration

   in one of three formats:

   legacy ( {source, target}-vocab-path) (pre-defined symbol ids)

   , resident (in memory - no config or pre-defined symbol ids)

   , or non-resident (database backed - db-config) - though we now load the whole thing into memory anyway.

   See xmt/VocabularyResource.hpp to
   instantiate a vocabulary from configuration
*/
struct VocabularyConfig {
  template <class Config>
  void configure(Config& config) {
    // TODO@SK: these could nest, and instead of format choosing which, figure out which one was configured by
    // user (boost::optional or initial null values)
    legacyConfig_.configure(config);
    residentConfig_.configure(config);
    nonResidentConfig_.configure(config);
    config("disable-reset", &disableReset)
        .deprecate()
        .defaulted()("has no effect; used to disable resetting caches");
    config(
        "Vocabulary configuration - in one of three formats: legacy ( {source, target}-vocab-path), resident "
        "(in memory - min, max), or non-resident (database backed - {id, sym}-to- {sym, id}-db)");
    config("format", &format)("(each format has its own options").defaulted();
    config.is("vocabulary");
    config(
        "Vocabulary configuration - in one of three formats: legacy ( {source, target}-vocab-path), resident "
        "(in memory - no config), or non-resident (database backed - db-config)");
  }

  friend inline std::ostream& operator<<(std::ostream& out, VocabularyConfig const& self) {
    self.print(out);
    return out;
  }
  bool usesNonResidentConfig() const { return format == kNon_Resident || format == kRead_Only; }
  bool isMdb() const { return usesNonResidentConfig() && nonResidentConfig_.isMdb(); }

  void print(std::ostream& out) const {
    if (usesNonResidentConfig())
      out << nonResidentConfig_;
    else if (format == kLegacy)
      out << legacyConfig_;
  }

  LegacyVocabularyConfig legacyConfig_;
  ResidentVocabularyConfig residentConfig_;
  NonResidentVocabularyConfig nonResidentConfig_;
  VocabularyFormat format = kResident;

  void validate() {}

 private:
  bool disableReset = false;
};

inline char const* resourceType(VocabularyConfig&) {
  return "vocabulary";
}


}

#endif
