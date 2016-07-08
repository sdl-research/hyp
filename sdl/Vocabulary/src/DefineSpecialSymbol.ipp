// Copyright 2014-2015 SDL plc
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
namespace sdl {
namespace Vocabulary {
namespace Symbols {
struct BOOST_PP_CAT(Symbol_, SDL_SPECIAL_SYMBOL_NAME)
    : SpecialSymbolTemplate<BOOST_PP_CAT(Symbol_, SDL_SPECIAL_SYMBOL_NAME)> {
  static char const* str() { return "<" SDL_SPECIAL_SYMBOL_TEXT ">"; }
  static const SymInt id = BOOST_PP_SLOT(1);
};
}
}
typedef Vocabulary::Symbols::BOOST_PP_CAT(Symbol_, SDL_SPECIAL_SYMBOL_NAME) SDL_SPECIAL_SYMBOL_NAME;
}
#define SDL_PP_EXPAND(x) x
#define SDL_INSTANTIATE_SPECIAL_SYMBOLS_TMP      \
  SDL_PP_EXPAND(SDL_INSTANTIATE_SPECIAL_SYMBOLS) \
  (void)sdl::BOOST_PP_CAT(Symbol_, SDL_SPECIAL_SYMBOL_NAME)::ID_LINKED; (void) sdl::BOOST_PP_CAT(Symbol_, SDL_SPECIAL_SYMBOL_NAME::TOKEN;
#undef SDL_INSTANTIATE_SPECIAL_SYMBOLS
#define SDL_INSTANTIATE_SPECIAL_SYMBOLS SDL_INSTANTIATE_SPECIAL_SYMBOLS_TMP

#define BOOST_PP_VALUE BOOST_PP_INC(BOOST_PP_SLOT(1))
#include BOOST_PP_ASSIGN_SLOT(1)
#undef SDL_SPECIAL_SYMBOL_NAME
#undef SDL_SPECIAL_SYMBOL_TEXT
