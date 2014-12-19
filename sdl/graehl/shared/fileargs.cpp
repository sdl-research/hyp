







#include <graehl/shared/gzstream.hpp>





















      set_new<ogzstream>(s, fail_msg);
    }
  } catch (std::exception &e) {
    fail_msg.append(" - exception: ").append(e.what());
    throw_fail(s, fail_msg);
  }
}










