//
// GLFW 3.5 Win32 - www.glfw.org
//

// This is a workaround for the fact that glfw3.h needs to export APIENTRY (for
// example to allow applications to correctly declare a GL_KHR_debug callback)
// but windows.h assumes no one will define APIENTRY before it does
#undef APIENTRY

#include <windows.h>

#define GLFW_WIN32_TLS_STATE            _GLFWtlsWin32     win32;
#define GLFW_WIN32_MUTEX_STATE          _GLFWmutexWin32   win32;

// Win32-specific thread local storage data
//
typedef struct _GLFWtlsWin32
{
    GLFWbool            allocated;
    DWORD               index;
} _GLFWtlsWin32;

// Win32-specific mutex data
//
typedef struct _GLFWmutexWin32
{
    GLFWbool            allocated;
    CRITICAL_SECTION    section;
} _GLFWmutexWin32;

