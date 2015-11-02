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
#pragma once
#ifndef GRAEHL_CPP11
#if __cplusplus >= 201103L || SDL_CPP11 || _MSC_VER >= 1900
#define GRAEHL_CPP11 1
#if __cplusplus >= 201400L
#define GRAEHL_CPP14 1
#endif
#else
#define GRAEHL_CPP11 0
#define GRAEHL_CPP14 0
#endif
#endif
