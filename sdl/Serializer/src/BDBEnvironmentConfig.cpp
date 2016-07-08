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
#include <sdl/Serializer/BDBEnvironmentConfig.hpp>
#include <sdl/Serializer/DbCxx.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/SetFlagBits.hpp>
#include <sdl/Util/TinyXml.hpp>
#include <sdl/ConfigStringLiterals.hpp>
#include <stdexcept>

namespace sdl {

void BDBEnvironmentConfig::setThread(bool thread) {
  setBdbProperty(envFlags, DB_THREAD, thread);
}

void BDBEnvironmentConfig::setCreate(bool create) {
  setBdbProperty(envFlags, DB_CREATE, create);
}

void BDBEnvironmentConfig::load(ticpp::Element* pConfig) {
  return doParse(pConfig);
}

void BDBEnvironmentConfig::doParse(ticpp::Element* xml) {
  using Util::xmlAttribute;
  std::string const kDefaultEnvPath;
  xmlAttribute(xml, SDL_CSTR_PATH, &envPath, kDefaultEnvPath);
  xmlAttribute(xml, SDL_CSTR_FLAGS, &envFlags, defaultEnvFlags());
  xmlAttribute(xml, SDL_CSTR_CACHESIZE, &cacheInMB, 100);
  xmlAttribute(xml, SDL_CSTR_ENCRYPT, &encrypt, false);
}

BdbEnvFlags BDBEnvironmentConfig::defaultEnvFlags() {
  // DB_PRIVATE | only this process may open environment
  // DB_INIT_MPOOL is generally necessary (for at least the first env you create?)
  return DB_PRIVATE | DB_INIT_MPOOL | DB_CREATE;
}
}


/**
    DB_INIT_CDB

Initialize locking for the Berkeley DB Concurrent Data Store product. In this mode, Berkeley DB provides
multiple reader/single writer access. The only other subsystem that should be specified with the DB_INIT_CDB
flag is DB_INIT_MPOOL.

 DB_INIT_LOCK

Initialize the locking subsystem. This subsystem should be used when multiple processes or threads are going
to be reading and writing a Berkeley DB database, so that they do not interfere with each other. If all
threads are accessing the database(s) read-only, locking is unnecessary. When the DB_INIT_LOCK flag is
specified, it is usually necessary to run a deadlock detector, as well. See db_deadlock and
DB_ENV->lock_detect() for more information.

 DB_INIT_LOG

Initialize the logging subsystem. This subsystem should be used when recovery from application or system
failure is necessary. If the log region is being created and log files are already present, the log files are
reviewed; subsequent log writes are appended to the end of the log, rather than overwriting current log
entries.

 DB_INIT_MPOOL

Initialize the shared memory buffer pool subsystem. This subsystem should be used whenever an application is
using any Berkeley DB access method.

 DB_INIT_REP

Initialize the replication subsystem. This subsystem should be used whenever an application plans on using
replication. The DB_INIT_REP flag requires the DB_INIT_TXN and DB_INIT_LOCK flags also be configured.

 DB_INIT_TXN

Initialize the transaction subsystem. This subsystem should be used when recovery and atomicity of multiple
operations are important. The DB_INIT_TXN flag implies the DB_INIT_LOG flag.

The second group of flags govern what recovery, if any, is performed when the environment is initialized:

 DB_RECOVER

Run normal recovery on this environment before opening it for normal use. If this flag is set, the DB_CREATE
and DB_INIT_TXN flags must also be set, because the regions will be removed and re-created, and transactions
are required for application recovery.

 DB_RECOVER_FATAL

Run catastrophic recovery on this environment before opening it for normal use. If this flag is set, the
DB_CREATE and DB_INIT_TXN flags must also be set, because the regions will be removed and re-created, and
transactions are required for application recovery.

A standard part of the recovery process is to remove the existing Berkeley DB environment and create a new one
in which to perform recovery. If the thread of control performing recovery does not specify the correct region
initialization information (for example, the correct memory pool cache size), the result can be an application
running in an environment with incorrect cache and other subsystem sizes. For this reason, the thread of
control performing recovery should specify correct configuration information before calling the DB_ENV->open()
method; or it should remove the environment after recovery is completed, leaving creation of the correctly
sized environment to a subsequent call to the DB_ENV->open() method.

All Berkeley DB recovery processing must be single-threaded; that is, only a single thread of control may
perform recovery or access a Berkeley DB environment while recovery is being performed. Because it is not an
error to specify DB_RECOVER for an environment for which no recovery is required, it is reasonable programming
practice for the thread of control responsible for performing recovery and creating the environment to always
specify the DB_CREATE and DB_RECOVER flags during startup.

The third group of flags govern file-naming extensions in the environment:

 DB_USE_ENVIRON

The Berkeley DB process' environment may be permitted to specify information to be used when naming files; see
Berkeley DB File Naming. Because permitting users to specify which files are used can create security
problems, environment information will be used in file naming for all users only if the DB_USE_ENVIRON flag is
set.

 DB_USE_ENVIRON_ROOT

The Berkeley DB process' environment may be permitted to specify information to be used when naming files; see
Berkeley DB File Naming. Because permitting users to specify which files are used can create security
problems, if the DB_USE_ENVIRON_ROOT flag is set, environment information will be used in file naming only for
users with appropriate permissions (for example, users with a user-ID of 0 on UNIX systems).

Finally, there are a few additional unrelated flags:

 DB_CREATE

Cause Berkeley DB subsystems to create any underlying files, as necessary.

 DB_LOCKDOWN

Lock shared Berkeley DB environment files and memory-mapped databases into memory.

 DB_FAILCHK

Internally call the DB_ENV->failchk() method as part of opening the environment. When DB_FAILCHK is specified,
a check is made to ensure all DB_ENV->failchk() prerequisites are meet.

If the DB_FAILCHK flag is used in conjunction with the DB_REGISTER flag, then a check will be made to see if
the environment needs recovery. If recovery is needed, a call will be made to the DB_ENV->failchk() method to
release any database reads locks held by the thread of control that exited and, if needed, to abort the
unresolved transaction. If DB_ENV->failchk() determines environment recovery is still required, the recovery
actions for DB_REGISTER will be followed.

If the DB_FAILCHK flag is not used in conjunction with the DB_REGISTER flag, then make an internal call to
DB_ENV->failchk() as the last step of opening the environment. If DB_ENV->failchk() determines database
environment recovery is required, DB_RUNRECOVERY will be returned.

 DB_PRIVATE

Allocate region memory from the heap instead of from memory backed by the filesystem or system shared memory.

This flag implies the environment will only be accessed by a single process (although that process may be
multithreaded). This flag has two effects on the Berkeley DB environment. First, all underlying data
structures are allocated from per-process memory instead of from shared memory that is accessible to more than
a single process. Second, mutexes are only configured to work between threads.

This flag should not be specified if more than a single process is accessing the environment because it is
likely to cause database corruption and unpredictable behavior. For example, if both a server application and
Berkeley DB utilities (for example, db_archive, db_checkpoint or db_stat) are expected to access the
environment, the DB_PRIVATE flag should not be specified.

See Shared Memory Regions for more information.

 DB_REGISTER

Check to see if recovery needs to be performed before opening the database environment. (For this check to be
accurate, all processes using the environment must specify DB_REGISTER when opening the environment.) If
recovery needs to be performed for any reason (including the initial use of the DB_REGISTER flag), and
DB_RECOVER is also specified, recovery will be performed and the open will proceed normally. If recovery needs
to be performed and DB_RECOVER is not specified, DB_RUNRECOVERY will be returned. If recovery does not need to
be performed, the DB_RECOVER flag will be ignored. See Architecting Transactional Data Store applications for
more information.

 DB_SYSTEM_MEM

Allocate region memory from system shared memory instead of from heap memory or memory backed by the
filesystem.

See Shared Memory Regions for more information.

 DB_THREAD

Cause the DB_ENV handle returned by DB_ENV->open() to be free-threaded; that is, concurrently usable by
multiple threads in the address space. The DB_THREAD flag should be specified if the DB_ENV handle will be
concurrently used by more than one thread in the process, or if any DB handles opened in the scope of the
DB_ENV handle will be concurrently used by more than one thread in the process.

This flag is required when using the Replication Manager..
*/
