#include <sdl/Util/InvalidInputException.hpp>
#include <sdl/graehl/shared/input_error.hpp>

namespace sdl { namespace Util {

void throwInvalidInputException(std::istream &in, std::string const& error, std::size_t itemNumber, const char *item) {
  graehl::throw_input_exception<InvalidInputException>(in, error, item, itemNumber);
}

}}
