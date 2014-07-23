#pragma once
/*
 * win.h
 *
 *  Created on: Jan 2, 2014
 */

#include "Common.h"

using WIN::ULONGLONG;
using WIN::CloseHandle;
using WIN::CreateMutex;
using WIN::FARPROC;
using WIN::GetCurrentProcess;
using WIN::GetCurrentProcessId;
using WIN::GetLastError;
using WIN::GetModuleHandleA;
using WIN::GetProcAddress;
using WIN::GetTickCount64;
using WIN::GetTickCount;
using WIN::GlobalFree;
using WIN::GlobalLock;
using WIN::HANDLE;
using WIN::HANDLE;
using WIN::HINSTANCE;
using WIN::IMAGEHLP_MODULE64;
using WIN::LIST_ENTRY;
using WIN::LoadLibraryA;
using WIN::LocalAlloc;
using WIN::LONG_PTR;    // Used by FIELD_OFFSET
using WIN::LONG;        // Used by FIELD_OFFSET
using WIN::MEMORY_BASIC_INFORMATION;
using WIN::PSYM_ENUMERATESYMBOLS_CALLBACK;
using WIN::ReleaseMutex;
using WIN::RtlpNumberOf; // ARRAYSIZE
using WIN::SINGLE_LIST_ENTRY;
using WIN::SYMBOL_INFO_PACKAGE;
using WIN::SymCleanup;
using WIN::SymEnumerateSymbols64;
using WIN::SymEnumSymbols;
using WIN::SymGetOptions;
using WIN::SymGetSearchPath;
using WIN::SymInitialize;
using WIN::SymLoadModule64;
using WIN::SymRefreshModuleList;
using WIN::SymSetOptions;
using WIN::SYMBOL_INFO;
using WIN::TCHAR;
using WIN::VirtualQuery;
using WIN::WaitForSingleObject;
