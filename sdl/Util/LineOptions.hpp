/** \file

    how to normalize a utf8 input line into a sequence of tokens - NFC,
    Windows-1252, corrupt utf8 handling, and whether to split it into words or
    characters.
*/

#ifndef LINEOPTIONS_JG_2013_12_16_HPP
#define LINEOPTIONS_JG_2013_12_16_HPP
#pragma once

#include <sdl/Util/NormalizeUtf8.hpp>
#include <sdl/Util/Trim.hpp>
#include <sdl/Util/Nfc.hpp>
#include <sdl/Util/Input.hpp>
#include <sdl/Util/Chomp.hpp>

namespace sdl {
namespace Util {

struct LineOptions : NormalizeUtf8 {

  bool chomp, trim;

  LineOptions(bool forceNfc = kXmtDefaultNfc) {
    defaults();
    nfc = forceNfc;
  }

  void defaults() {
    chomp = true;
    trim = false;
  }

  template <class Config>
  void configure(Config& config) {
    NormalizeUtf8::configure(config);
    config.is("LineOptions");
    config("chomp", &chomp).self_init();
    config("trim", &trim)('t').self_init()(
        "remove initial and trailing whitespace from each line first (otherwise just remove endline if "
        "'chomp')");
  }

  void normalize(std::string& str) const {
    NormalizeUtf8::normalize(str);
    if (trim)
      Util::trim(str);
    else if (chomp)
      Util::chomp(str);
  }

  bool getlineNormalized(std::istream& in, std::string& line, char until = '\n') const {
    if ((bool)std::getline(in, line, until)) {
      normalize(line);
      return true;
    } else
      return false;
  }

  bool getline(Util::InputStream& in, std::string& line, char until = '\n') const {
    return getlineNormalized(*in, line, until);
  }

  bool operator()(Util::InputStream& in, std::string& line, char until = '\n') const {
    return getlineNormalized(*in, line, until);
  }

  bool operator()(std::istream& in, std::string& line, char until = '\n') const {
    return getlineNormalized(in, line, until);
  }
};

extern LineOptions nfcline;


}}

#endif
