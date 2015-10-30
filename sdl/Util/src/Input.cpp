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

#define GRAEHL__GZSTREAM_MAIN
#define GRAEHL__RANDOM_MAIN

#include <sdl/Util/Input.hpp>
#include <graehl/shared/random.hpp>
#include <sdl/Util/LineOptions.hpp>
#include <sdl/Util/File.hpp>

#if SDL_ENCRYPT
#include <sdl/Encrypt/Encrypt.hpp>
#include <sdl/Util/File.hpp>
#endif

namespace sdl {
namespace Util {

LineOptions nfcline;
LineOptions nfclineAlways(true);
LineOptions nfclineWarnOnly(false, false);

void Input::init(std::string const& filename, bool mayDecrypt, bool allowNullFile) {
  decrypted.reset();
  using namespace std;
  if (!mayDecrypt || special_input_filename(filename) || Util::endsWith(filename, kGzSuffix)
      || Util::startsWith(filename, kPipePrefix) || Util::startsWith(filename, kPipePrefix2)) {
    set(filename);
  } else {
#if SDL_ENCRYPT
    shared_ptr<ifstream> f(new ifstream);
    Encrypt::openBinary(*f, filename);
    decrypted.reset(new string);
    if (Encrypt::maybeDecryptFile(*f, *decrypted, true))
      set(new StringArrayStream(decrypted), filename);
    else {
      decrypted.reset();
      this->setPtr(f, filename);
    }
#else
    set(filename);
#endif
  }
}

shared_ptr<std::string> const& Input::decryptedString() {
  if (!decrypted) {
    decrypted.reset(new std::string);
#if SDL_ENCRYPT
    Encrypt::decrypt(getStream(), *decrypted);
#else
    Util::readFileContents(getStream(), *decrypted);
#endif
  }
  return decrypted;
}

bool Input::nonseekable() const {
  return special_input_filename(name) || Util::endsWith(name, kGzSuffix) || Util::startsWith(name, kPipePrefix);
}

void Input::decryptIfFile() {
#if SDL_ENCRYPT
  if (!decrypted) {
    if (!nonseekable()) {
      decrypted.reset(new std::string);
      if (Encrypt::maybeDecryptIfFile(getStream(), *decrypted)) {
        set(new StringArrayStream(decrypted), name);
        return;
      }
    }
    decrypted.reset();
  }
#else
  if (!nonseekable()) {
    std::istream& in = getStream();
    if (Util::isFile(in)) {
      decrypted.reset(new std::string);
      Util::readFileContentsSeekTo(dynamic_cast<std::ifstream&>(in), *decrypted);
    }
  }
#endif
}


}}
