// GLFW 3.2 Win32 - www.glfw.org

#include "internal.h"
/// GLFW internal API

// Initialise timer
//
void _glfwInitTimerWin32(void)
{   uint64_t frequency;
    if (QueryPerformanceFrequency((LARGE_INTEGER*) &frequency))
    {   _glfw.win32_time.hasPC = GLFW_TRUE;
        _glfw.win32_time.frequency = frequency;
    } else
    {   _glfw.win32_time.hasPC = GLFW_FALSE;
        _glfw.win32_time.frequency = 1000;
    }
}

/// GLFW platform API

uint64_t _glfwPlatformGetTimerValue(void)
{   if (_glfw.win32_time.hasPC)
    {   uint64_t value;
        QueryPerformanceCounter((LARGE_INTEGER*) &value);
           return value;
    } else return (uint64_t) _glfw_timeGetTime();
}
uint64_t _glfwPlatformGetTimerFrequency(void)
{ return _glfw.win32_time.frequency;
}
