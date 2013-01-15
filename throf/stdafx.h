// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define _CRT_SECURE_NO_WARNINGS

#ifdef _WIN32
#include <windows.h>
#else
#define UNREFERENCED_PARAMETER(e) e

#if ((__GNUC__ <= 4) && (__GNUC_MINOR__ < 6))
#define nullptr NULL
#endif

#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>

// STL stoofs
#include <stdexcept>
#include <string>
#include <sstream>
#include <stack>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <regex>
#include <cctype>
#include <memory>
#include <functional>
#include <exception>
#include <algorithm>

#define STRINGIFY(e) #e
#define printInfo(s, ...) ::printf("INFO: " s "\n", __VA_ARGS__)
#define printError(s, ...) ::printf("ERROR: " s "\n", __VA_ARGS__)

#include "throfexception.h"
#include "common.h"
#include "tokenizer.h"
#include "stackelement.h"
#include "interpreter.h"

// TODO: reference additional headers your program requires here
