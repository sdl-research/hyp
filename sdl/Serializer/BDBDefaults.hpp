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
#ifndef BDBDEFAULTS_JG20121120_HPP
#define BDBDEFAULTS_JG20121120_HPP
#pragma once

/** \file

   Since we fetch all the records for a given Key in one go we can never
   estimate the size of the buffer that should be allocated. If this value is too
   big, it becomes hard for the OS to find large blocks of contiguous memory thus
   bringing swap space into play and slowing down the entire application.  If this
   value is too small we end up invkoking malloc / free multiple times and thus
   fragmanting the memory space and wasting CPU cycles. Users should be careful
   about what this values is set as.  TODO TODO@SK Identify appropriate values for
   Vocabulary(should be small as no duplicate records.) and Rules DB (fairly large
   as we can possibly have a lot of duplicate records).

*/

namespace sdl {

/// default initial buffer size. should be configured depending on type of db. if insufficient, grow
/// exponentially until max
static const unsigned kDefaultBufferSize = 8192;

/// default upper limit on buffer size for all values matching a key - for grammars, may need to be large.
static const unsigned kDefaultMaxBufferSize = 1073741824;  // (1gb)

/// not user configured
static const unsigned kDefaultPageSize = 4096;


}

#endif
