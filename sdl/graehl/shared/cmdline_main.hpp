#ifndef GRAEHL_SHARED__CMDLINE_MAIN_HPP
#define GRAEHL_SHARED__CMDLINE_MAIN_HPP


/* yourmain inherit from graehl::main (
   overriding:
   virtual run()
   set_defaults() maybe calling set_defaults_base() etc.)
   then INT_MAIN(yourmain)










*/











#ifndef GRAEHL_CONFIG_FILE

#endif
#ifndef GRAEHL_IN_FILE

#endif
#ifndef GRAEHL_OUT_FILE

#endif
#ifndef GRAEHL_LOG_FILE

#endif

#ifndef GRAEHL_DEBUGPRINT
# define GRAEHL_DEBUGPRINT 0
#endif











#include <graehl/shared/program_options.hpp>
#include <graehl/shared/itoa.hpp>
#include <graehl/shared/command_line.hpp>
#include <graehl/shared/fileargs.hpp>
#include <graehl/shared/teestream.hpp>
#include <graehl/shared/string_to.hpp>
#include <graehl/shared/random.hpp>

#if GRAEHL_DEBUGPRINT
# include <graehl/shared/debugprint.hpp>
#endif
#include <iostream>





namespace graehl {




struct base_options {
  char const* positional_help() const {
    return positional_in?"; positional args ok too":"";
  }
  std::string input_help() const {
    std::ostringstream s;
    if (add_ins()) {
      s << "Multiple input files (- for STDIN) at least " << min_ins;
      if (has_max_ins())
        s << " and at most "+itos(max_ins);
    } else if (add_in_file)
      s << "Input file (- for STDIN)";
    else
      return "No input file options";
    s << positional_help();
    return s.str();
  }

  bool add_ins() const {
    return min_ins || max_ins;
  }
  bool has_max_ins() const {
    return max_ins>=0;
  }
  void allow_ins(bool positional = true, int max_ins_=-1) {
    max_ins = max_ins_;
    positional_in = positional;
  }
  void allow_in(bool positional = true) {
    add_in_file = true;
    positional_in = positional;
  }
  void require_ins(bool positional = true, int max_ins_=-1) {
    allow_ins(positional, max_ins_);
    min_ins = 1;
  }



      validate(n);
      for (unsigned i = 0; i<n; ++i)
        if (!*ins[i]) {
          throw std::runtime_error("Invalid input file #"+utos(i+1)+" file "+ins[i].name);
        }
    }

  void validate(int n) const {
    if (has_max_ins() && n>max_ins)
      throw std::runtime_error("Too many input files (max="+itos(max_ins)+", had "+itos(n)+")");
    if (n<min_ins)
      throw std::runtime_error("Too few input files (min="+itos(min_ins)+", had "+itos(n)+")");
  }
  void allow_random(bool val = true)
  {
    add_random = val;
  }
  int min_ins, max_ins; //multiple inputs 0,1,... if max>min
  bool add_in_file, add_out_file, add_log_file, add_config_file, add_help, add_debug_level;
  bool positional_in, positional_out;
  bool add_quiet, add_verbose;
  bool add_random;
  base_options() {
    positional_out = positional_in = add_in_file = add_random = false;
    add_log_file = add_help = add_out_file = add_config_file = add_debug_level = true;
    min_ins = max_ins = 0;


  }
};































    if (random)
      allow_random();
    if (input) {
      if (multifile) {









































  }

  std::string get_version() const
  {

  }




  int debug_lvl;
  bool help;
  bool quiet;
  int verbose;
  ostream_arg log_file, out_file;
  istream_arg in_file, config_file;







  std::string cmdname, cmdline_str;
  std::ostream *log_stream;
  std::auto_ptr<teebuf> teebufptr;
  std::auto_ptr<std::ostream> teestreamptr;

  boost::uint32_t random_seed;















    verbose = 1;
    random_seed = default_random_seed();
    out_file = stdout_arg();
    log_file = stderr_arg();
    in_file = stdin_arg();


  }























  }

  typedef printable_options_description<std::ostream> OD;













  void print_version(std::ostream &o) {

  }







  virtual void run()
  {
    print_version(out());
  }

  virtual void set_defaults()
  {
    set_defaults_base();
    set_defaults_extra();
  }

  virtual void validate_parameters()
  {
    validate_parameters_base();
    validate_parameters_extra();
    set_random_seed(random_seed);
  }

  virtual void validate_parameters_extra() {}


  {
    if (options_added)
      return;



  }




  {

    if (general.size())

    if (cosmetic.size())


    options_added = true;
  }

  virtual void log_invocation()
  {
    if (verbose>0)
      log_invocation_base();
  }







  virtual void set_defaults_extra() {}

  inline std::ostream &log() const {
    return *log_stream;
  }

  inline std::istream &in() const {
    return *in_file;
  }

  inline std::ostream &out() const {
    return *out_file;
  }





  void log_invocation_base()
  {
    log() << "### COMMAND LINE:\n" << cmdline_str << "\n";
    log() << "### USING OPTIONS:\n";







  }


  void validate_parameters_base()
  {


    log_stream = log_file.get();
    if (!log_stream)
      log_stream=&std::cerr;
    else if (!is_default_log(log_file)) {
      // tee to cerr
      teebufptr.reset(new teebuf(log_stream->rdbuf(), std::cerr.rdbuf()));
      teestreamptr.reset(log_stream = new std::ostream(teebufptr.get()));
    }


    if (in_file && ins.empty())
      ins.push_back(in_file);

#if GRAEHL_DEBUGPRINT && defined(DEBUG)
    DBP::set_logstream(&log());
    DBP::set_loglevel(log_level);
#endif

  }














































  {



          ("help,h", boost::program_options::bool_switch(&help),
           "show usage/documentation")
          ;



          ("quiet,q", boost::program_options::bool_switch(&quiet),
           "use log only for warnings - e.g. no banner of command line options used")
          ;



          ("verbose,v", defaulted_value(&verbose),
           "e.g. verbosity level >1 means show banner of command line options. 0 means don't")
          ;



          (GRAEHL_OUT_FILE",o", defaulted_value(&out_file),
           "Output here (instead of STDOUT)");

        output_positional();
    }



          (GRAEHL_IN_FILE",i", optional_value(&ins)->multitoken(),

           )
          ;




          (GRAEHL_IN_FILE",i", defaulted_value(&in_file),


        input_positional();
    }



          (GRAEHL_CONFIG_FILE, optional_value(&config_file),
           "load boost program options config from file");



          (GRAEHL_LOG_FILE",l", defaulted_value(&log_file),
           "Send log messages here (as well as to STDERR)");

#if GRAEHL_DEBUGPRINT


          ("debug-level,d", defaulted_value(&debug_lvl),

#endif



          ("random-seed,R", defaulted_value(&random_seed),
           "Random seed")
          ;








































  }




























  boost::program_options::variables_map vm;
  boost::program_options::positional_options_description positional;













  void add_positional(std::string const& name, int n = 1) {

  }
  void input_positional(int n = 1) {

  }
  void output_positional() {

  }
  void log_positional() {

  }
  void all_positional() {
    input_positional();
    output_positional();
    log_positional();
  }




  {
























    cmdline_str = graehl::get_command_line(argc, argv, NULL);

    try {

















        return false;
      }

        try {

          //NOTE: this means that cmdline opts have precedence. hooray.
          config_file.close();


          throw;
        }
      }

      if (help) {

        return false;
      }

    } catch (std::exception &e) {

      throw;
    }
    return true;
  }






  //FIXME: defaults cannot change after first parse_args

  {

    try {
      if (!parse_args(argc, argv))

      validate_parameters();
      log_invocation();

    }

      return carpexcept("ran out of memory\nTry descreasing -m or -M, and setting an accurate -P if you're using initial parameters.");
    }
    catch(std::exception& e) {
      return carpexcept(e.what());
    }
    catch(char const* e) {
      return carpexcept(e);
    }
    catch(...) {
      return carpexcept("Exception of unknown type!");
    }
    return 0;
  }

  // for some reason i see segfault with log() when exiting from main. race condition? weird.
  template <class C>
  int carpexcept(C const& c) const
  {

    return 1;
  }
  template <class C>
  int carp(C const& c) const
  {

    return 1;
  }
  template <class C>
  void warn(C const& c) const
  {

  }

  virtual ~main() {}
};






}















#endif
