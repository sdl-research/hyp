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

    fstream utilities
*/

#ifndef SDL_UTIL__FILE_JG_2014_02_12_HPP
#define SDL_UTIL__FILE_JG_2014_02_12_HPP
#pragma once

#include <fstream>
#include <sdl/Exception.hpp>
#include <sdl/Util/LogHelper.hpp>

namespace sdl { namespace Util {


/**
   seek f to position len(prefix) and \return true file starts with 'prefix',
   else seek to beginning (if rewind) and \return false. if that seek fails,
   throw.
*/
inline bool fileStartsWith(std::ifstream &f, std::string const& prefix, bool rewind) {
  f.seekg(0, std::ios::beg);
  using std::string;
  string::size_type const sz = prefix.size();
  string buf(sz, '\0');
  if (f.read((char*)buf.data(), sz) && prefix == buf)
    return true;
  else
    f.clear();
  if (rewind) {
    if (!f.seekg(0, std::ios::beg))
      SDL_THROW_LOG(Util.fileStartsWith, FileException, "couldn't rewind to start of file without header: " << prefix);
  }
  return false;
}

inline bool isFile(std::istream &in) {
  return dynamic_cast<std::ifstream *>(&in);
}

inline void readFileContentsNonseekable(std::istream &in, std::string& content) {
  std::ostringstream ostrm;
  ostrm << in.rdbuf();
  content = ostrm.str();
}

/**
   set out = contents of f starting at byte startAtByte.
*/
inline void readFileContentsSeekTo(std::ifstream &f, std::string &out, std::size_t startAtByte = 0, std::size_t minSize = 0) {
  f.seekg(0, std::ios::end);
  if (!f)
    SDL_THROW_LOG(File, FileException, "can't advance unencrypted file to get size");

  std::size_t fileSize = f.tellg();
  std::size_t sz = fileSize - startAtByte;
  if (startAtByte > fileSize || sz < minSize)
    SDL_THROW_LOG(File, FileFormatException,
                  "file not big enough - expected " << minSize << " bytes or more after " << startAtByte << " header");
  f.seekg(startAtByte, std::ios::beg);
  if (!f)
    SDL_THROW_LOG(File, FileException, "can't rewind file to " << startAtByte);
  out.resize(sz);
  if (!f.read(arrayBegin(out), sz))
    SDL_THROW_LOG(File, FileException, "can't read remaining " << sz << " bytes of file");
}

/**
   set out = contents of f (first seeking to beginning if f is a file stream)
*/
inline void readFileContents(std::istream& in, std::string& content)
{
  if (isFile(in))
    readFileContentsSeekTo(static_cast<std::ifstream&>(in), content, 0);
  else
    Util::readFileContentsNonseekable(in, content);
}

}}

#endif
