#include <iostream>

#include <iomanip>
#include <fstream>
#include <streambuf>
#include <string>








































































namespace Config {





































/*



























 */




}






  }






































      }
      os << " ]";
    }




      os << " { ";
    }









        }



        else

      }

    }






    }
  }
}


ConfigNode loadRawEncryptedConfig(boost::filesystem::path const& path) {
  try {



    YAML::Node rawNode = YAML::Load(decryptedtext);

    if (is_null(rawNode))  // because it turns out that YAML::LoadFile doesn't throw if file can't be read!

    return rawNode;




  }
}


ConfigNode loadRawConfig(boost::filesystem::path const& path) {
  try {
    ConfigNode rawNode = YAML::LoadFile(path.string());


    return rawNode;




  }
}

/// fully expanded starting from relative or absolute path 'basis',
/// and interpreted via conventions specified in ../docs/xmt-configuration-with-yaml.pdf


  configProc.setFilePath(filePath);

}














  }
}



