














#include <log4cxx/xml/domconfigurator.h>



namespace po = boost::program_options;



int main(int argc, char** argv) {

  po::options_description generic("Generic options");






  po::positional_options_description p;
  p.add("input-file", 1);
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(generic).positional(p).run(), vm);
  po::notify(vm);
  if (vm.count("help")) {


    return EXIT_SUCCESS;
  }

  Util::Input input;
  if (vm.count("input-file")) {


  }


  std::istream& instream = input.getStream();
  typedef Hypergraph::ArcTpl<Hypergraph::ViterbiWeightTpl<float> > Arc;

  Hypergraph::IHypergraphsIteratorTpl<Arc>* pHgIter



  while (!pHgIter->done()) {
    Hypergraph::IHypergraph<Arc>* pHg = pHgIter->value();
    pHgIter->next();
    Hypergraph::printHgAsMosesLattice(*pHg);
  }
}
