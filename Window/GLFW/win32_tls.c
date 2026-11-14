// GLFW 3.2 Win32 - www.glfw.org

#include "internal.h"

/// GLFW internal API

GLFWbool _glfwInitThreadLocalStorageWin32(void)
{
    _glfw.win32_tls.context = TlsAlloc();
    if (_glfw.win32_tls.context == TLS_OUT_OF_INDEXES)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Win32: Failed to allocate TLS index");
        return GLFW_FALSE;
    }

    _glfw.win32_tls.allocated = GLFW_TRUE;
    return GLFW_TRUE;
}

void _glfwTerminateThreadLocalStorageWin32(void)
{
    if (_glfw.win32_tls.allocated)
        TlsFree(_glfw.win32_tls.context);
}

/// GLFW platform API

void _glfwPlatformSetCurrentContext(_GLFWwindow* context)
{
    TlsSetValue(_glfw.win32_tls.context, context);
}

_GLFWwindow* _glfwPlatformGetCurrentContext(void)
{
    return TlsGetValue(_glfw.win32_tls.context);
}
