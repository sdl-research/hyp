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
/** \file

    a base class for xmt exceptions (Exception)

    usage:
    VERBOSE_EXCEPTION_DECLARE(EmptySetException)

    SDL_THROW0(EmptySetException);
    SDL_THROW2(EmptySetException, "because", b); // space separated args

    see also: Subdir/Exception.hpp

    see also: Util/LogHelper for SDL_THROW_LOG
*/

#ifndef SDL_BASE_EXCEPTION_HPP
#define SDL_BASE_EXCEPTION_HPP
#pragma once

#include <exception>
#include <graehl/shared/verbose_exception.hpp>

#define SDL_THROW0(n) VTHROW_A(n)
#define SDL_THROW(n, why) VTHROW_A_1(n, why)
#define SDL_THROW2(n, why, a2) VTHROW_A_2(n, why, a2)
#define SDL_THROW3(n, why, a2, a3) VTHROW_A_3(n, why, a2, a3)
#define SDL_THROW4(n, why, a2, a3, a4) VTHROW_A_4(n, why, a2, a3, a4)
#define SDL_THROW_BRIEF(n, why) throw n(why)

namespace graehl {
struct string_to_exception;
}

namespace sdl {

typedef graehl::verbose_exception Exception;
typedef graehl::string_to_exception ConfigStringException;

typedef std::out_of_range OutOfRange;
// prefer throwing this to IndexException for consistency with e.g. vector.at(i)
// and for performance in high-impact calls like Sym lookup

VERBOSE_EXCEPTION_DECLARE(EmptySetException)
VERBOSE_EXCEPTION_DECLARE(CycleException)
VERBOSE_EXCEPTION_DECLARE(UnimplementedException)
VERBOSE_EXCEPTION_DECLARE(LogNegativeException)
VERBOSE_EXCEPTION_DECLARE(SelfModifyException)
VERBOSE_EXCEPTION_DECLARE(BadTypeConstantException)
VERBOSE_EXCEPTION_DECLARE(NotANumberException)
VERBOSE_EXCEPTION_DECLARE(FileException)
VERBOSE_EXCEPTION_DECLARE(FileFormatException)
VERBOSE_EXCEPTION_DECLARE(IOException)
VERBOSE_EXCEPTION_DECLARE(ConfigException)
VERBOSE_EXCEPTION_DECLARE(RuleFormatException)
VERBOSE_EXCEPTION_DECLARE(BDBException)
VERBOSE_EXCEPTION_DECLARE(IndexException)
VERBOSE_EXCEPTION_DECLARE(DuplicateException)
VERBOSE_EXCEPTION_DECLARE(ProgrammerMistakeException)
VERBOSE_EXCEPTION_DECLARE(PostConditionException)
VERBOSE_EXCEPTION_DECLARE(BufferOverrunException)
VERBOSE_EXCEPTION_DECLARE(HadoopException)

/// for errno using system calls that return -1 on failure - see Util::throw_if_errno(ret, name)
VERBOSE_EXCEPTION_DECLARE(OSException)
VERBOSE_EXCEPTION_DECLARE(TypeException)
VERBOSE_EXCEPTION_DECLARE(TodoException)
VERBOSE_EXCEPTION_DECLARE(OutOfMemoryException)
VERBOSE_EXCEPTION_DECLARE(TimeoutException)

VERBOSE_EXCEPTION_DECLARE(MissingInterimDataException)
VERBOSE_EXCEPTION_DECLARE(InterimDataTypeException)
VERBOSE_EXCEPTION_DECLARE(TokenizeException)
VERBOSE_EXCEPTION_DECLARE(ParseException)
VERBOSE_EXCEPTION_DECLARE(EntityException)
VERBOSE_EXCEPTION_DECLARE(InvalidInputException)
VERBOSE_EXCEPTION_DECLARE(DeprecatedException)

VERBOSE_EXCEPTION_DECLARE(ModuleRegistrationException)
VERBOSE_EXCEPTION_DECLARE(ResourceRegistrationException)

VERBOSE_EXCEPTION_DECLARE(MissingResourceException)
VERBOSE_EXCEPTION_DECLARE(UnknownModuleTypeException)

VERBOSE_EXCEPTION_DECLARE(TrainableCapitalizerModuleException)
VERBOSE_EXCEPTION_DECLARE(LowerCaserModuleException)


}

#endif
