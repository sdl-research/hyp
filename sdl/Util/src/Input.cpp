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

LineOptions nfcline(true);

void Input::init(std::string const& filename, bool mayDecrypt, bool allowNullFile) {
  decrypted.reset();
  SDL_TRACE(Util.Input, "Reading input file '" << filename << "'");
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

sdl::shared_ptr<std::string> const& Input::decryptedString() {
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
