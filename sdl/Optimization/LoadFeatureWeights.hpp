




namespace Optimization {

template<class Map>
void loadFeatureWeightsFile(std::string const& filename, Map* pMap) {


  typename Map::key_type featureId;
  typename Map::mapped_type featureWeight;

  std::size_t linenum = 1;
  while (inStream >> featureId) {
    if (inStream.eof()) {

                    filename << ":" << linenum << ": No feature weight found for ID " << featureId);
    }
    inStream >> featureWeight;
    (*pMap)[featureId] = featureWeight;
    ++linenum;
  }

}



#endif
