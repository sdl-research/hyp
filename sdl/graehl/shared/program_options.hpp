










#ifndef GRAEHL__SHARED__PROGRAM_OPTIONS_HPP
#define GRAEHL__SHARED__PROGRAM_OPTIONS_HPP















#ifdef _WIN32
#include <iso646.h>
#endif




#include <boost/program_options.hpp>



#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <stdexcept>
#include <fstream>
#include <boost/pool/object_pool.hpp>

#include <graehl/shared/prefix_option.hpp>










namespace graehl {































































































































template <class V>
inline bool maybe_get(boost::program_options::variables_map const& vm, std::string const& key, V& val) {
  if (vm.count(key)) {
    val = vm[key].as<V>();
    return true;
  }
  return false;
}

inline std::string get_string(boost::program_options::variables_map const& vm, std::string const& key) {
  return vm[key].as<std::string>();
}




  if (!*s || *s++ != '-') return 0;
  if (!*s || *s++ != '-') return 0;
  int chars_replaced = 0;
  for (; *s; ++s) {

    if (*s == '-') {
      *s = '_';
      ++chars_replaced;
    }
  }
  return chars_replaced;
}


  int chars_replaced = 0;
  for (int i = 1; i < argc; ++i) {
    chars_replaced += arg_minusto_underscore(argv[i]);
  }
  return chars_replaced;
}







































































  return boost::program_options::value<T>(v);
}














template <class T>



}

template <class T>


}




  throw std::runtime_error(msg);
}



  boost::program_options::validators::check_first_occurrence(v);
  return boost::program_options::validators::get_single_string(values);
}

template <class I>

  char c;


}

template <class Ostream>

  typedef boost::function<void(Ostream&, boost::any const&)> F;

  template <class T>








    o << *boost::any_cast<T const>(&t);
  }

  any_printer() {}



  template <class T>



  template <class T>


    swap(f);
  }
};












// have to wrap regular options_description and store our own tables because
// author didn't make enough stuff protected/public or add a virtual print
// method to value_semantic
template <class Ostream>

  typedef printable_options_description<Ostream> self_type;
  typedef boost::program_options::options_description options_description;
  typedef boost::program_options::option_description option_description;
  typedef boost::shared_ptr<self_type> group_type;
  typedef std::vector<group_type> groups_type;

    typedef boost::shared_ptr<option_description> OD;

    any_printer<Ostream> print;
    OD od;
    bool in_group;







    template <class T>


    printable_option() : in_group(false) {}
  };
  typedef std::vector<printable_option> options_type;





  typedef boost::object_pool<std::string> string_pool;





  void init() {
    n_this_level = 0;
    n_nonempty_groups = 0;
    descs.reset(new string_pool());
  }





  template <class V>



  }

  template <class V>


  }


  template <class V>



  }

  template <class V>











  }

  boost::shared_ptr<string_pool> descs;  // because opts lib only takes char *, hold them here.
  template <class T, class C>


    return (*this)(name, val, cstr(description), hidden);
  }



  template <class T, class C>


    return (*this)(cstr(name), val, cstr(description), hidden);
  }

  std::size_t n_this_level, n_nonempty_groups;
  template <class T, class C>



    ++n_this_level;
    printable_option opt((T*)0, simple_add(name, val, description));

    return *this;
  }




    options_description::add(desc);
    if (hidden) return *this;
    groups.push_back(group_type(new self_type(desc)));
    if (desc.size()) {
      for (typename options_type::const_iterator i = desc.pr_options.begin(), e = desc.pr_options.end();
           i != e; ++i) {
        pr_options.push_back(*i);
        pr_options.back().in_group = true;
      }
      ++n_nonempty_groups;  // could just not add an empty group. but i choose to allow that.
    }

    return *this;
  }



    using namespace boost;
    using namespace boost::program_options;
    using namespace std;
    string const& name = opt.name();
    if (!only_value) {



      if (var.empty()) {
        o << "#EMPTY# " << name;
        return;
      }
      o << name << " = ";
    }
    opt.print(o, var.value());
  }
























    const bool show_defaulted = bool(show_flags & SHOW_DEFAULTED);
    const bool show_description = bool(show_flags & SHOW_DESCRIPTION);
    const bool hierarchy = bool(show_flags & SHOW_HIERARCHY);
    const bool show_empty = bool(show_flags & SHOW_EMPTY);
    const bool show_help = bool(show_flags & SHOW_HELP);
    const bool show_empty_groups = bool(show_flags & SHOW_EMPTY_GROUPS);

    using namespace boost::program_options;
    using namespace std;





      variable_value const& var = vm[opt.vmkey()];



      print_option(o, opt, var);

    }

    if (hierarchy)


  }

  typedef std::vector<std::string> unparsed_args;

















  // remember to call store(return, vm) and notify(vm)
  boost::program_options::parsed_options



    using namespace boost::program_options;
    command_line_parser cl(argc, const_cast<char**>(argv));
    cl.options(*this);


    parsed_options parsed = cl.run();









      if (!allow_unrecognized_positional) {



    }





    return parsed;
  }

  /// parses arguments, then stores/notifies from opts->vm.  returns unparsed
  /// options and positional arguments, but if not empty, throws exception unless
  /// allow_unrecognized_positional is true
  std::vector<std::string>

                           boost::program_options::positional_options_description* po = NULL,

    unparsed_args r;


    notify(vm);
    return r;
  }

  std::size_t ngroups() const { return groups.size(); }
  std::size_t size() const { return pr_options.size(); }

 private:
  groups_type groups;
  options_type pr_options;
  std::string caption;

                                                   const boost::program_options::value_semantic* s,

    typedef option_description OD;

    options_description::add(od);
    return od;
  }
};

typedef printable_options_description<std::ostream> printable_opts;






















}  // graehl







































































#endif
