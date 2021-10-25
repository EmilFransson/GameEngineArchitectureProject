#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <WinUser.h>
#include <wrl/client.h>

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>
#include <DirectXMath.h>

#include <string>
#include <memory>
#include <stdint.h>
#include <crtdbg.h>
#include <assert.h>
#include <iostream>
#include <comdef.h>
#include <vector>
#include <chrono>
#include <cstddef>
#include <array>
#include <thread>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <mutex>
#include <map>
#include <Objbase.h>
#include <fstream>
#include <filesystem>
#include <cmath>

#include <mutex>
#include <thread>
#include <functional>
#include <deque>

// Iostream - STD I/O Library
//#include <iostream>

// Vector - STD Vector/Array Library
//#include <vector>

// String - STD String Library
//#include <string>

// fStream - STD File I/O Library
//#include <fstream>

// Math.h - STD math Library
#include <math.h>

#if defined(DEBUG) | defined (_DEBUG)
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else 
#define DBG_NEW new 
#endif