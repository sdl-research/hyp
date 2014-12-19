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
