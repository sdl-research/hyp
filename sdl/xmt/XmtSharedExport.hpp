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

    allows use of select symbols with windows dynamic linking (.DLL)

    XMT_API void exportedFn(); // in .hpp

    void exportedFn() { } // in.cpp.

    class XMT_API ExportedClass { // in.hpp
    };
*/

#ifndef XMT_SHAREDEXPORT_HPP
#define XMT_SHAREDEXPORT_HPP
#pragma once

// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the xmt_shared_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// XMT_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#ifdef _WIN32
#ifdef xmt_shared_EXPORTS  // Defined by default in MSVC to match the project's name
#define XMT_API __declspec(dllexport)
#else
#define XMT_API
#endif
#else
#define XMT_API
#endif

#endif
