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
#include <sdl/Db/BDBWrapperConfig.hpp>
#include <sdl/Db/BdbFlags.hpp>
#include <sdl/Serializer/BDBDefaults.hpp>
#include <sdl/Serializer/DbCxx.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/SetFlagBits.hpp>
#include <sdl/Util/TinyXml.hpp>
#include <sdl/ConfigStringLiterals.hpp>

namespace sdl {

SDL_NAME_ENUM(BdbIndexType);

bool BDBWrapperConfig::hasDuplicates() const {
  return indexTypeAllowsDuplicates() && (setFlags & (DB_DUP | DB_DUPSORT));
}

void BDBWrapperConfig::validateDuplicates() {
  BdbSetFlags rightsort = sortDuplicates ? DB_DUPSORT : DB_DUP;
  BdbSetFlags wrongsort = sortDuplicates ? DB_DUP : DB_DUPSORT;

  if (setFlags & wrongsort)
    SDL_WARN(Db.BDBConfig, *this << " had the wrong sort of duplicates request: " << wrongsort
                                 << " - changing to " << rightsort
                                 << " as is appropriate for sort-duplicates: " << sortDuplicates);

  setBdbProperty(setFlags, wrongsort, false);
  setBdbProperty(setFlags, rightsort, duplicates);
}

void BDBWrapperConfig::setThread(bool thread) {
  setBdbProperty(openFlags, DB_THREAD, thread);
}

void BDBWrapperConfig::setEncrypt(bool encrypt) {
  // TODO: test
  setBdbProperty(setFlags, DB_ENCRYPT, encrypt);
}

bool BDBWrapperConfig::creating() const {
  return openFlags & DB_CREATE;
}

void BDBWrapperConfig::setCreate(bool create) {
  setBdbProperty(openFlags, DB_CREATE, create);
  if (create) openFlags &= ~DB_RDONLY;
}

void BDBWrapperConfig::setExcl(bool excl) {
  setBdbProperty(openFlags, DB_EXCL, excl);
}


void BDBWrapperConfig::setReadOnly() {
  openFlags |= DB_RDONLY;
  openFlags &= ~DB_CREATE;
  openFlags &= ~DB_EXCL;
}

void BDBWrapperConfig::load(ticpp::Element* xml) {
  using namespace Util;
  xmlAttribute(xml, SDL_CSTR_MAXATTEMPTS, &maxAttempts, 5u);
  // file and name could be passed in later
  xmlAttribute(xml, SDL_CSTR_FILEPATH, &dbFile);
  xmlAttribute(xml, SDL_CSTR_NAME, &dbName);
  // these defaults are defined above; may be overridden
  xmlAttribute(xml, SDL_CSTR_BUFFERSIZE, &bufferSize, (unsigned)kDefaultBufferSize);
  xmlAttribute(xml, SDL_CSTR_PAGESIZE, &pageSize, (unsigned)kDefaultPageSize);
  xmlAttribute(xml, SDL_CSTR_TYPE, &indexType, kHash);
  xmlAttribute(xml, SDL_CSTR_MAXBUFFERSIZE, &maxBufferSize, (unsigned)kDefaultMaxBufferSize);
  openFlags = DB_RDONLY;
  maybeXmlAttribute(xml, SDL_CSTR_FLAGS, &openFlags);
  setFlags = 0;
  maybeXmlAttribute(xml, SDL_CSTR_ADDITIONALFLAGS, &setFlags);
  validate();
}


}
