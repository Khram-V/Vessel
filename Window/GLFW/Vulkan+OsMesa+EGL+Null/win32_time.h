
// GLFW 3.5 Win32 - www.glfw.org

// This is a workaround for the fact that glfw3.h needs to export APIENTRY (for
// example to allow applications to correctly declare a GL_KHR_debug callback)
// but windows.h assumes no one will define APIENTRY before it does
#undef APIENTRY

#include <windows.h>

#define GLFW_WIN32_LIBRARY_TIMER_STATE  _GLFWtimerWin32   win32;

// Win32-specific global timer data
//
typedef struct _GLFWtimerWin32
{
    uint64_t            frequency;
} _GLFWtimerWin32;

