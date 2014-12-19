#include <iostream>
#include <sstream>
#include <string>







#ifndef NDEBUG

#endif











namespace po = boost::program_options;





int main(int ac, char** av) {
  try {

    po::options_description generic("Allowed options");




    po::options_description hidden("Hidden options");


    po::options_description cmdline_options;
    cmdline_options.add(generic).add(hidden);

    po::options_description config_file_options;
    config_file_options.add(generic).add(hidden);

    po::positional_options_description p;
    p.add("input-file", -1);

    po::variables_map vm;


    if (vm.count("config-file")) {


      notify(vm);
    }

    if (vm.count("help")) {

      std::cout << generic << "\n";
      return EXIT_FAILURE;
    }

    std::string file;



    using namespace Hypergraph;

    typedef ViterbiWeightTpl<float> Weight;




    Util::Input in_(file);

    MutableHypergraph<Arc> hg;
    hg.setVocabulary(pVoc);


    writeOpenFstFormat(std::cout, hg);

    assert(hg.checkValid());


    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
