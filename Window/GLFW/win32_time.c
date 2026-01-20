//
// GLFW 3.5 Win32 - www.glfw.org
//

#include "internal.h"

#if defined(GLFW_BUILD_WIN32_TIMER)

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _glfwPlatformInitTimer(void)
{
    QueryPerformanceFrequency((LARGE_INTEGER*) &_glfw.timer.win32.frequency);
}

uint64_t _glfwPlatformGetTimerValue(void)
{
    uint64_t value;
    QueryPerformanceCounter((LARGE_INTEGER*) &value);
    return value;
}

uint64_t _glfwPlatformGetTimerFrequency(void)
{
    return _glfw.timer.win32.frequency;
}

#endif // GLFW_BUILD_WIN32_TIMER

