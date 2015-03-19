/** \file

    Input from stdin or filename as an istream smartpointer (.gz extension = gunzipped transparently).

    - for STDIN/STDOUT, -2 for STDERR, -0 for none, x.gz for gzipped
*/

#ifndef SDL_LWUTIL_INPUT_HPP
#define SDL_LWUTIL_INPUT_HPP
#pragma once


#include <vector>
#include <boost/any.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <graehl/shared/fileargs.hpp>
#include <graehl/shared/string_match.hpp>
#include <sdl/SharedPtr.hpp>
#include <sdl/Util/ArrayStream.hpp>
#include <sdl/Util/String.hpp>

namespace sdl {
namespace Util {

typedef graehl::file_arg<std::istream> InputStream;
typedef graehl::file_arg<std::ostream> OutputStream;

using graehl::stdin_arg;
using graehl::stdout_arg;
using graehl::special_input_filename;
using graehl::gz_filename;
using graehl::stdin_filename;
using graehl::stdout_filename;
using graehl::stderr_filename;
using graehl::null_filename;

char const* const kStdoutFilename("-");
char const* const kStderrFilename("-2");
char const* const kNullFilename("-0");
std::string const kNullFilenameStr((kNullFilename));

inline bool isNullFilename(std::string const& path) {
  return path == kNullFilenameStr;
}

/// no need to check isNullFilename
inline bool isNullFile(InputStream const& in) {
  return !in;
}

inline bool isNullFile(OutputStream const& in) {
  return !in;
}

inline InputStream inputStream(std::istream &in, std::string const& name) {
  return InputStream(in, name);
}

inline bool moreChars(std::istream &in, char &c) {
  if (!in) return false;
  return (bool)(in >> c);
}

inline void throwIfMoreChars(std::istream &in, std::string const& why) {
  char c;
  if (moreChars(in, c))
    SDL_THROW_LOG(Input, FileFormatException, why << " read unexpected character " << c);
}

/**
   return if str is one of kStderrFilename kStdoutFilename or kNullFilename. these are not to be yaml basis path expanded
*/
inline bool isNonFilesystemPath(std::string const& str) {
  return !str.empty() && str[0]=='-' &&
      (str.size()==1 || (str.size()==2 && (str[1]=='2'||str[1]=='0')));
}


/**
   for per-id (per-request) output. for experimentation, not production. production should have the option of keeping all intermediate results in memory (with configurable debugging/recovery serialization)
*/
static std::string fileForId(std::string prefix, std::string const& id, std::string const& sep="") {
  if (prefix.empty() || id.empty() || prefix==kStdoutFilename || prefix==kStderrFilename || prefix==kNullFilenameStr)
    return prefix;
  std::string gzsuffix(".gz");
  if (graehl::strip_suffix(prefix, gzsuffix))
    return prefix+sep+id+gzsuffix;
  return prefix+sep+id;
}

static std::string fileForIdHelp() {
  return graehl::file_arg_usage()+"; if empty, nothing; otherwise, for each segment id filename.gz -> filename.id.gz or filename -> filename.id";
}

namespace {
std::string const& kGzSuffix(".gz");
std::string const& kPipePrefix("/proc/self/fd/");
std::string const& kPipePrefix2("/dev/fd/");
}

/**
   Represents input, either from STDIN (default, or filename '-'), or
   from a file if setFilename is called (or constructed with string
   argument). filename '-0' means no file.

   an Input can be used as an InputStream (so like shared_ptr<istream>) that default
   constructs to stdin instead of no input

   both InputStream and Input are configurable (via configure library or program_options directly)
*/
struct Input : InputStream {
  typedef void leaf_configure; // configure.hpp
  friend std::string const& to_string_impl(Input const& fa) {
    return fa.name;
  }

  friend void string_to_impl(std::string const& name, Input & fa) {
    fa.init(name);
  }

  friend char const* type_string(Input const&) { return "Input filename (.gz, '-', or encrypted)"; }

  static std::string help() {
    return graehl::file_arg_usage();
  }

  static std::string helpId() {
    return fileForIdHelp();
  }

  Input() : InputStream(stdin_filename) {
    SDL_DEBUG(Util.Input, "Reading input from stdin");
  }

  /**
     STDIN (default, or filename '-'), or
      from a file if setFilename is called (or constructed with string
      argument). filename '-0' means no file.
     */
  Input(std::string const& filename, bool mayDecrypt = true, bool allowNullFile = true) {
    if (allowNullFile && filename.empty())
      set_none();
    else
      init(filename, mayDecrypt, allowNullFile);
  }

  Input(std::istream &in, std::string const& filename, bool mayDecrypt = true) : InputStream(in, filename) {
    if (mayDecrypt)
      decryptIfFile();
  }

  void init(std::string const& filename, bool mayDecrypt = true, bool allowNullFile = true);

  Input(InputStream const& in, bool mayDecrypt = true) {
    init(in, mayDecrypt);
  }

  Input(Input const& o)
      : InputStream(o)
      , decrypted(o.decrypted)
  {}

  void operator = (Input const& o) {
    InputStream::operator = ((InputStream const&)o);
    decrypted = o.decrypted;
  }

  void init(InputStream const& in, bool mayDecrypt = true) {
    decrypted.reset();
    InputStream::operator = (in);
    if (mayDecrypt)
      decryptIfFile();
  }

  void decryptIfFile();

  Input(std::string const& filenamePrefix, std::string const& id) : InputStream(fileForId(filenamePrefix, id)) {}

  /**
     set to null. this is different from -0, which presents a valid writable stream (targeted at something like /dev/null)
  */
  void setNull() {
    decrypted.reset();
    InputStream::set_none();
  }

  void clear() {
    setNull();
  }

  void reset() {
    setNull();
  }

  operator bool() const {
    return this->valid();
  }

  std::string getFilename() const {
    return InputStream::str();
  }

  std::istream& getStream() const {
    return *(*this);
  }

  /**
     if file wasn't encrypted, return string of file contents. if it was, return decrypted string.
  */
  shared_ptr<std::string> const& decryptedString();

  shared_ptr<std::string> decrypted;

  /**
     \return whether known nonseekable filename (.gz, stdin, etc).
  */
  bool nonseekable() const;

};

inline Input stdinInput() {
  return Input(stdin_filename);
}


// Reads a file (possibly decrypting it) into a string.
// Should only be used for small files.
inline sdl::shared_ptr<std::string> readFile(std::string const& filename) {
  Util::Input in(filename, false);
  return in.decryptedString();
}

}}

namespace boost { namespace program_options {
inline void validate(boost::any& v,
                     const std::vector<std::string>& values,
                     sdl::Util::Input* target_type, int)
{
  v = boost::any(sdl::Util::Input(graehl::get_single_arg(v, values)));
}
}} // boost::program_options

#endif
