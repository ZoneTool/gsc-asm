// ======================= ZoneTool =======================
// zonetool, a fastfile linker for various
// Call of Duty titles. 
//
// Project: https://github.com/ZoneTool/gsc-asm
// Author: RektInator (https://github.com/RektInator)
// License: GNU GPL v3.0
// ========================================================
#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <array>
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <fstream>
#include <stdio.h>
#include <tchar.h>

#define IW5
#ifdef IW5
#include "IW5_PC.hpp"
#elif defined(IW6)
#include "IW6_PC.hpp"
#endif
