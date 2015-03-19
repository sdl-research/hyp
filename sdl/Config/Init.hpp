/** \file

     for constructors that wish to use the .init specified in .configure()
     methods. for faster compiles, this is self-contained

   usage:

   struct YourConfigurable {
     int val;

     template <class Config>
     void configure(Config &c) {
       c("val", &val).init(1);
     }

     YourConfigurable() { Config::init(this); } // this sets val=1 per the .init in configure above

   };

*/

#ifndef INIT_JG_2013_05_23_HPP
#define INIT_JG_2013_05_23_HPP
#pragma once

#include <graehl/shared/configure_init.hpp>

namespace sdl {
namespace Config {

/**
   usage: Config::inits(this) - uses the .init in your configure(config) (recursively to init sub-items)
*/
template <class Val>
void inits(Val* val) {
  configure::init_expr<Val> config(val);
  val->configure(config);
}


}}

#endif
