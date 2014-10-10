#pragma once

#include <string>
#include <sstream>
#include <cstdio>
#include <map>
#include <set>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>

#include "pin.H"

namespace WIN
{
#include <Windows.h>
#include <dbghelp.h>
#include <avrfsdk.h>
}

using namespace std;

#include "win.h"
#include "pinlock.h"
#include "logfile.h"
#include "log.h"
#include "timearray.h"
#include "md5.h"
#include "sortable-mixin.h"
#include "stringutil.h"

#include "knobs.h"
#include "data-type.h"

#include "instrumenter.h"
#include "address-instrumenter.h"
#include "named-image-instrumenter.h"
#include "symbolic-resolver.h"

#include "symbol.h"
#include "hook.h"
#include "allocator.h"
#include "page-heap.h"
#include "heap-handle.h"
#include "symbolic-heap.h"
#include "whitelist.h"
#include "stack.h"
#include "thread.h"
#include "memory-metadata.h"

#include "image-logger.h"
#include "instruction-logger.h"
#include "image-symbol-loader.h"
#include "image-whitelister.h"
#include "log-memory.h"
#include "log-branch.h"
#include "stack-instrumenter.h"

#include "ida-splode.h"
