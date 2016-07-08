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

 .
*/

#ifndef DB_GRAEHL_2016_07_07_HPP
#define DB_GRAEHL_2016_07_07_HPP
#pragma once

#ifdef _WIN32
#ifdef _WINSOCKAPI_
#undef _WINSOCKAPI_
#include <db.h>
#define _WINSOCKAPI_ 1
#endif
#else
#include <db.h>
#endif


#endif
