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
#ifndef SDL_OPTIMIZATION_LOADFEATUREWEIGHTS_HPP
#define SDL_OPTIMIZATION_LOADFEATUREWEIGHTS_HPP
#pragma once

namespace sdl {
namespace Optimization {

template<class Map>
void loadFeatureWeightsFile(std::string const& filename, Map* pMap) {
  SDL_INFO(Optimization, "Loading feature weights from '" << filename << "'");
  Util::Input input(filename);
  typename Map::key_type featureId;
  typename Map::mapped_type featureWeight;
  std::istream& inStream = *input;
  std::size_t linenum = 1;
  while (inStream >> featureId) {
    if (inStream.eof()) {
      SDL_THROW_LOG(Optimization, ParseException,
                    filename << ":" << linenum << ": No feature weight found for ID " << featureId);
    }
    inStream >> featureWeight;
    (*pMap)[featureId] = featureWeight;
    ++linenum;
  }
  SDL_INFO(Optimization, "Loaded " << (linenum - 1) << " feature weights from '" << filename << "'");
}

}}

#endif
