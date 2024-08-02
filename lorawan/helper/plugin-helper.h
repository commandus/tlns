//
// Created by andrei on 02.08.24.
//

#ifndef TLNS_PLUGIN_HELPER_H
#define TLNS_PLUGIN_HELPER_H

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <Windows.h>
#define dlopen(fileName, opt) LoadLibraryA(fileName)
#define dlclose FreeLibrary
#define dlsym GetProcAddress
#define PLUGIN_FILE_NAME_SUFFIX ".dll"
#define EXPORT_SHARED_C_FUNC extern "C" __declspec(dllexport)
#else
typedef void * HINSTANCE;
#include <dlfcn.h>
#include <algorithm>
#define PLUGIN_FILE_NAME_SUFFIX ".so"
#define EXPORT_SHARED_C_FUNC extern "C"
#endif

#endif //TLNS_PLUGIN_HELPER_H
