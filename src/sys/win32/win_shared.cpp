/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/




#include "win_local.h"
#include <lmerr.h>
#include <lmcons.h>
#include <lmwksta.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <direct.h>
#include <io.h>
#include <conio.h>

#ifndef	ID_DEDICATED
#ifndef INT_MAX
#define INT_MAX 2147483647
#endif
#ifndef INT_MIN
#define INT_MIN (-2147483647 - 1)
#endif
#include <comdef.h>
#include <comutil.h>
#include <Wbemidl.h>

#pragma comment (lib, "wbemuuid.lib")
#endif

#include <stdint.h>

/*
================
Sys_Milliseconds
================
*/
int Sys_Milliseconds(void) {
	int sys_curtime;
	static int sys_timeBase;
	static bool	initialized = false;

	if (!initialized) {
		sys_timeBase = timeGetTime();
		initialized = true;
	}
	sys_curtime = timeGetTime() - sys_timeBase;

	return sys_curtime;
}

/*
========================
Sys_Microseconds
========================
*/
uint64_t Sys_Microseconds(void) {
	static uint64_t ticksPerMicrosecondTimes1024 = 0;

	if (ticksPerMicrosecondTimes1024 == 0) {
		ticksPerMicrosecondTimes1024 = ((uint64_t)Sys_ClockTicksPerSecond() << 10) / 1000000;
		assert(ticksPerMicrosecondTimes1024 > 0);
	}

	return ((uint64_t)((int64_t)Sys_GetClockTicks() << 10)) / ticksPerMicrosecondTimes1024;
}


/*
================
Sys_GetSystemRam

	returns amount of physical memory in MB
================
*/
int Sys_GetSystemRam(void) {
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);
	int physRam = statex.ullTotalPhys / (1024 * 1024);
	// HACK: For some reason, ullTotalPhys is sometimes off by a meg or two, so we round up to the nearest 16 megs
	physRam = (physRam + 8) & ~15;
	return physRam;
}


/*
================
Sys_GetDriveFreeSpace
returns in megabytes
================
*/
int Sys_GetDriveFreeSpace(const char* path) {
	DWORDLONG lpFreeBytesAvailable;
	DWORDLONG lpTotalNumberOfBytes;
	DWORDLONG lpTotalNumberOfFreeBytes;
	int ret = 26;
	//FIXME: see why this is failing on some machines
	if (::GetDiskFreeSpaceEx(path, (PULARGE_INTEGER)&lpFreeBytesAvailable, (PULARGE_INTEGER)&lpTotalNumberOfBytes, (PULARGE_INTEGER)&lpTotalNumberOfFreeBytes)) {
		ret = (double)(lpFreeBytesAvailable) / (1024.0 * 1024.0);
	}
	return ret;
}


/*
================
Sys_GetVideoRam
returns in megabytes
================
*/
int Sys_GetVideoRam(void) {
	return 100 * 1024 * 1024;
}

/*
================
Sys_GetCurrentMemoryStatus

	returns OS mem info
	all values are in kB except the memoryload
================
*/
void Sys_GetCurrentMemoryStatus(sysMemoryStats_t& stats) {
	MEMORYSTATUSEX statex;
	unsigned __int64 work;

	memset(&statex, sizeof(statex), 0);
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);

	memset(&stats, 0, sizeof(stats));

	stats.memoryLoad = statex.dwMemoryLoad;

	work = statex.ullTotalPhys >> 20;
	stats.totalPhysical = *(int*)&work;

	work = statex.ullAvailPhys >> 20;
	stats.availPhysical = *(int*)&work;

	work = statex.ullAvailPageFile >> 20;
	stats.availPageFile = *(int*)&work;

	work = statex.ullTotalPageFile >> 20;
	stats.totalPageFile = *(int*)&work;

	work = statex.ullTotalVirtual >> 20;
	stats.totalVirtual = *(int*)&work;

	work = statex.ullAvailVirtual >> 20;
	stats.availVirtual = *(int*)&work;

	work = statex.ullAvailExtendedVirtual >> 20;
	stats.availExtendedVirtual = *(int*)&work;
}

/*
================
Sys_LockMemory
================
*/
bool Sys_LockMemory(void* ptr, int bytes) {
	return (VirtualLock(ptr, (SIZE_T)bytes) != FALSE);
}

/*
================
Sys_UnlockMemory
================
*/
bool Sys_UnlockMemory(void* ptr, int bytes) {
	return (VirtualUnlock(ptr, (SIZE_T)bytes) != FALSE);
}

/*
================
Sys_SetPhysicalWorkMemory
================
*/
void Sys_SetPhysicalWorkMemory(int minBytes, int maxBytes) {
	::SetProcessWorkingSetSize(GetCurrentProcess(), minBytes, maxBytes);
}

/*
================
Sys_GetCurrentUser
================
*/
char* Sys_GetCurrentUser(void) {
	static char s_userName[1024];
	unsigned long size = sizeof(s_userName);


	if (!GetUserName(s_userName, &size)) {
		strcpy(s_userName, "player");
	}

	if (!s_userName[0]) {
		strcpy(s_userName, "player");
	}

	return s_userName;
}


/*
===============================================================================

	Call stack

===============================================================================
*/


#define PROLOGUE_SIGNATURE 0x00EC8B55

#include <dbghelp.h>

const int UNDECORATE_FLAGS = UNDNAME_NO_MS_KEYWORDS |
UNDNAME_NO_ACCESS_SPECIFIERS |
UNDNAME_NO_FUNCTION_RETURNS |
UNDNAME_NO_ALLOCATION_MODEL |
UNDNAME_NO_ALLOCATION_LANGUAGE |
UNDNAME_NO_MEMBER_TYPE;

#if defined(_DEBUG) && 1

typedef struct symbol_s {
	int					address;
	char* name;
	struct symbol_s* next;
} symbol_t;

typedef struct module_s {
	int					address;
	char* name;
	symbol_t* symbols;
	struct module_s* next;
} module_t;

module_t* modules;

/*
==================
SkipRestOfLine
==================
*/
void SkipRestOfLine(const char** ptr) {
	while ((**ptr) != '\0' && (**ptr) != '\n' && (**ptr) != '\r') {
		(*ptr)++;
	}
	while ((**ptr) == '\n' || (**ptr) == '\r') {
		(*ptr)++;
	}
}

/*
==================
SkipWhiteSpace
==================
*/
void SkipWhiteSpace(const char** ptr) {
	while ((**ptr) == ' ') {
		(*ptr)++;
	}
}

/*
==================
ParseHexNumber
==================
*/
int ParseHexNumber(const char** ptr) {
	int n = 0;
	while ((**ptr) >= '0' && (**ptr) <= '9' || (**ptr) >= 'a' && (**ptr) <= 'f') {
		n <<= 4;
		if (**ptr >= '0' && **ptr <= '9') {
			n |= ((**ptr) - '0');
		}
		else {
			n |= 10 + ((**ptr) - 'a');
		}
		(*ptr)++;
	}
	return n;
}

/*
==================
Sym_Init
==================
*/
void Sym_Init(long addr) {
	TCHAR moduleName[MAX_STRING_CHARS];
	MEMORY_BASIC_INFORMATION mbi;

	VirtualQuery((void*)addr, &mbi, sizeof(mbi));

	GetModuleFileName((HMODULE)mbi.AllocationBase, moduleName, sizeof(moduleName));

	char* ext = moduleName + strlen(moduleName);
	while (ext > moduleName && *ext != '.') {
		ext--;
	}
	if (ext == moduleName) {
		strcat(moduleName, ".map");
	}
	else {
		strcpy(ext, ".map");
	}

	module_t* module = (module_t*)malloc(sizeof(module_t));
	module->name = (char*)malloc(strlen(moduleName) + 1);
	strcpy(module->name, moduleName);
	module->address = (int)mbi.AllocationBase;
	module->symbols = NULL;
	module->next = modules;
	modules = module;

	FILE* fp = fopen(moduleName, "rb");
	if (fp == NULL) {
		return;
	}

	int pos = ftell(fp);
	fseek(fp, 0, SEEK_END);
	int length = ftell(fp);
	fseek(fp, pos, SEEK_SET);

	char* text = (char*)malloc(length + 1);
	fread(text, 1, length, fp);
	text[length] = '\0';
	fclose(fp);

	const char* ptr = text;

	// skip up to " Address" on a new line
	while (*ptr != '\0') {
		SkipWhiteSpace(&ptr);
		if (idStr::Cmpn(ptr, "Address", 7) == 0) {
			SkipRestOfLine(&ptr);
			break;
		}
		SkipRestOfLine(&ptr);
	}

	int symbolAddress;
	int symbolLength;
	char symbolName[MAX_STRING_CHARS];
	symbol_t* symbol;

	// parse symbols
	while (*ptr != '\0') {

		SkipWhiteSpace(&ptr);

		ParseHexNumber(&ptr);
		if (*ptr == ':') {
			ptr++;
		}
		else {
			break;
		}
		ParseHexNumber(&ptr);

		SkipWhiteSpace(&ptr);

		// parse symbol name
		symbolLength = 0;
		while (*ptr != '\0' && *ptr != ' ') {
			symbolName[symbolLength++] = *ptr++;
			if (symbolLength >= sizeof(symbolName) - 1) {
				break;
			}
		}
		symbolName[symbolLength++] = '\0';

		SkipWhiteSpace(&ptr);

		// parse symbol address
		symbolAddress = ParseHexNumber(&ptr);

		SkipRestOfLine(&ptr);

		symbol = (symbol_t*)malloc(sizeof(symbol_t));
		symbol->name = (char*)malloc(symbolLength);
		strcpy(symbol->name, symbolName);
		symbol->address = symbolAddress;
		symbol->next = module->symbols;
		module->symbols = symbol;
	}

	free(text);
}

/*
==================
Sym_Shutdown
==================
*/
void Sym_Shutdown(void) {
	module_t* m;
	symbol_t* s;

	for (m = modules; m != NULL; m = modules) {
		modules = m->next;
		for (s = m->symbols; s != NULL; s = m->symbols) {
			m->symbols = s->next;
			free(s->name);
			free(s);
		}
		free(m->name);
		free(m);
	}
	modules = NULL;
}

/*
==================
Sym_GetFuncInfo
==================
*/
void Sym_GetFuncInfo(long addr, idStr& module, idStr& funcName) {
	MEMORY_BASIC_INFORMATION mbi;
	module_t* m;
	symbol_t* s;

	VirtualQuery((void*)addr, &mbi, sizeof(mbi));

	for (m = modules; m != NULL; m = m->next) {
		if (m->address == (int)mbi.AllocationBase) {
			break;
		}
	}
	if (!m) {
		Sym_Init(addr);
		m = modules;
	}

	for (s = m->symbols; s != NULL; s = s->next) {
		if (s->address == addr) {

			char undName[MAX_STRING_CHARS];
			if (UnDecorateSymbolName(s->name, undName, sizeof(undName), UNDECORATE_FLAGS)) {
				funcName = undName;
			}
			else {
				funcName = s->name;
			}
			for (int i = 0; i < funcName.Length(); i++) {
				if (funcName[i] == '(') {
					funcName.CapLength(i);
					break;
				}
			}
			module = m->name;
			return;
		}
	}

	sprintf(funcName, "0x%08x", addr);
	module = "";
}

#elif defined(_DEBUG)

DWORD lastAllocationBase = -1;
HANDLE processHandle;
idStr lastModule;

/*
==================
Sym_Init
==================
*/
void Sym_Init(long addr) {
	TCHAR moduleName[MAX_STRING_CHARS];
	TCHAR modShortNameBuf[MAX_STRING_CHARS];
	MEMORY_BASIC_INFORMATION mbi;

	if (lastAllocationBase != -1) {
		Sym_Shutdown();
	}

	VirtualQuery((void*)addr, &mbi, sizeof(mbi));

	GetModuleFileName((HMODULE)mbi.AllocationBase, moduleName, sizeof(moduleName));
	_splitpath(moduleName, NULL, NULL, modShortNameBuf, NULL);
	lastModule = modShortNameBuf;

	processHandle = GetCurrentProcess();
	if (!SymInitialize(processHandle, NULL, FALSE)) {
		return;
	}
	if (!SymLoadModule(processHandle, NULL, moduleName, NULL, (DWORD)mbi.AllocationBase, 0)) {
		SymCleanup(processHandle);
		return;
	}

	SymSetOptions(SymGetOptions() & ~SYMOPT_UNDNAME);

	lastAllocationBase = (DWORD)mbi.AllocationBase;
}

/*
==================
Sym_Shutdown
==================
*/
void Sym_Shutdown(void) {
	SymUnloadModule(GetCurrentProcess(), lastAllocationBase);
	SymCleanup(GetCurrentProcess());
	lastAllocationBase = -1;
}

/*
==================
Sym_GetFuncInfo
==================
*/
void Sym_GetFuncInfo(long addr, idStr& module, idStr& funcName) {
	MEMORY_BASIC_INFORMATION mbi;

	VirtualQuery((void*)addr, &mbi, sizeof(mbi));

	if ((DWORD)mbi.AllocationBase != lastAllocationBase) {
		Sym_Init(addr);
	}

	BYTE symbolBuffer[sizeof(IMAGEHLP_SYMBOL) + MAX_STRING_CHARS];
	PIMAGEHLP_SYMBOL pSymbol = (PIMAGEHLP_SYMBOL)&symbolBuffer[0];
	pSymbol->SizeOfStruct = sizeof(symbolBuffer);
	pSymbol->MaxNameLength = 1023;
	pSymbol->Address = 0;
	pSymbol->Flags = 0;
	pSymbol->Size = 0;

	DWORD symDisplacement = 0;
	if (SymGetSymFromAddr(processHandle, addr, &symDisplacement, pSymbol)) {
		// clean up name, throwing away decorations that don't affect uniqueness
		char undName[MAX_STRING_CHARS];
		if (UnDecorateSymbolName(pSymbol->Name, undName, sizeof(undName), UNDECORATE_FLAGS)) {
			funcName = undName;
		}
		else {
			funcName = pSymbol->Name;
		}
		module = lastModule;
	}
	else {
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR)&lpMsgBuf,
			0,
			NULL
		);
		LocalFree(lpMsgBuf);

		// Couldn't retrieve symbol (no debug info?, can't load dbghelp.dll?)
		sprintf(funcName, "0x%08x", addr);
		module = "";
	}
}

#else

/*
==================
Sym_Init
==================
*/
void Sym_Init(long addr) {
}

/*
==================
Sym_Shutdown
==================
*/
void Sym_Shutdown(void) {
}

/*
==================
Sym_GetFuncInfo
==================
*/
void Sym_GetFuncInfo(long addr, idStr& module, idStr& funcName) {
	module = "";
	sprintf(funcName, "0x%08x", addr);
}

#endif

/*
==================
Sys_ShutdownSymbols
==================
*/
void Sys_ShutdownSymbols(void) {
	Sym_Shutdown();
}
