


#include <fstream>
#include <iostream>
#include <sstream>
#include <string>







#ifndef NDEBUG

#endif





















namespace po = boost::program_options;




































    }





  }










  hg.setVocabulary(pVoc);

  parseText(*in, file, &hg);









  }

  typedef typename Arc::Weight Weight;








}






  try {





    po::options_description generic("Allowed options");











    po::options_description cmdline_options;


    po::options_description config_file_options;


    po::positional_options_description p;
    p.add("input-file", -1);

    po::variables_map vm;




    if (vm.count("config-file")) {


    }






      std::cout << generic << "\n";
      return EXIT_FAILURE;
    }




    if (arcType == "log") {
      typedef ArcTpl<LogWeightTpl<float> > Arc;





      typedef ArcTpl<ExpectationWeight> Arc;







    }


    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
