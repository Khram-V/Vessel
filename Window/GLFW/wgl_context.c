// GLFW 3.2 WGL - www.glfw.org

#include "internal.h"
#include <stdlib.h>
#include <malloc.h>

// Return a list of available and usable framebuffer configs
//
static GLFWbool choosePixelFormat(_GLFWwindow* window,
                                  const _GLFWfbconfig* desired,
                                  int* result)
{
    _GLFWfbconfig* usableConfigs;
    const _GLFWfbconfig* closest;
    int i, nativeCount, usableCount;
    {
        nativeCount = DescribePixelFormat(window->context.wgl.dc,
                                          1,
                                          sizeof(PIXELFORMATDESCRIPTOR),
                                          NULL);
    }
    usableConfigs = calloc(nativeCount, sizeof(_GLFWfbconfig));
    usableCount = 0;

    for (i = 0;  i < nativeCount;  i++)
    {
        const int n = i + 1;
        _GLFWfbconfig* u = usableConfigs + usableCount;
        { PIXELFORMATDESCRIPTOR pfd;
            // Get pixel format attributes through legacy PFDs
            if (!DescribePixelFormat(window->context.wgl.dc,
                                     n,
                                     sizeof(PIXELFORMATDESCRIPTOR),
                                     &pfd))
            {
                continue;
            }

            if (!(pfd.dwFlags & PFD_DRAW_TO_WINDOW) ||
                !(pfd.dwFlags & PFD_SUPPORT_OPENGL))
            {
                continue;
            }

            if (!(pfd.dwFlags & PFD_GENERIC_ACCELERATED) &&
                (pfd.dwFlags & PFD_GENERIC_FORMAT))
            {
                continue;
            }

            if (pfd.iPixelType != PFD_TYPE_RGBA)
                continue;

            u->redBits = pfd.cRedBits;
            u->greenBits = pfd.cGreenBits;
            u->blueBits = pfd.cBlueBits;
            u->alphaBits = pfd.cAlphaBits;

            u->depthBits = pfd.cDepthBits;
            u->stencilBits = pfd.cStencilBits;

            u->accumRedBits = pfd.cAccumRedBits;
            u->accumGreenBits = pfd.cAccumGreenBits;
            u->accumBlueBits = pfd.cAccumBlueBits;
            u->accumAlphaBits = pfd.cAccumAlphaBits;

            u->auxBuffers = pfd.cAuxBuffers;

            if (pfd.dwFlags & PFD_STEREO)
                u->stereo = GLFW_TRUE;
            if (pfd.dwFlags & PFD_DOUBLEBUFFER)
                u->doublebuffer = GLFW_TRUE;
        }

        u->handle = n;
        usableCount++;
    }

    if (!usableCount)
    {
        _glfwInputError(GLFW_API_UNAVAILABLE,
                        "WGL: The driver does not appear to support OpenGL");

        free(usableConfigs);
        return GLFW_FALSE;
    }

    closest = _glfwChooseFBConfig(desired, usableConfigs, usableCount);
    if (!closest)
    {
        _glfwInputError(GLFW_FORMAT_UNAVAILABLE,
                        "WGL: Failed to find a suitable pixel format");

        free(usableConfigs);
        return GLFW_FALSE;
    }

    *result = (int) closest->handle;
    free(usableConfigs);

    return GLFW_TRUE;
}

// Returns whether desktop compositing is enabled
//
static GLFWbool isCompositionEnabled(void)
{
    BOOL enabled;

    if (!_glfw_DwmIsCompositionEnabled)
        return FALSE;

    if (_glfw_DwmIsCompositionEnabled(&enabled) != S_OK)
        return FALSE;

    return enabled;
}

static void makeContextCurrent(_GLFWwindow* window)
{
    if (window)
    {
        if (wglMakeCurrent(window->context.wgl.dc, window->context.wgl.handle))
            _glfwPlatformSetCurrentContext(window);
        else
        {
            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "WGL: Failed to make context current");
            _glfwPlatformSetCurrentContext(NULL);
        }
    }
    else
    {
        if (!wglMakeCurrent(NULL, NULL))
        {
            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "WGL: Failed to clear current context");
        }

        _glfwPlatformSetCurrentContext(NULL);
    }
}

static void swapBuffers(_GLFWwindow* window)
{
    // HACK: Use DwmFlush when desktop composition is enabled
    if (isCompositionEnabled() && !window->monitor)
    {
        int count = abs(window->context.wgl.interval);
        while (count--)
            _glfw_DwmFlush();
    }

    SwapBuffers(window->context.wgl.dc);
}

static void swapInterval(int interval)
{
    _GLFWwindow* window = _glfwPlatformGetCurrentContext();

    window->context.wgl.interval = interval;

    // HACK: Disable WGL swap interval when desktop composition is enabled to
    //       avoid interfering with DWM vsync
    if (isCompositionEnabled() && !window->monitor)interval = 0;
//***** if (_glfw.wgl.EXT_swap_control)_glfw.wgl.SwapIntervalEXT(interval);
}

static GLFWglproc getProcAddress(const char* procname)
{
    const GLFWglproc proc = (GLFWglproc) wglGetProcAddress(procname);
    if (proc)
        return proc;

    return (GLFWglproc) GetProcAddress(_glfw.wgl.instance, procname);
}

// Destroy the OpenGL context
//
static void destroyContext(_GLFWwindow* window)
{
    if (window->context.wgl.handle)
    {
        wglDeleteContext(window->context.wgl.handle);
        window->context.wgl.handle = NULL;
    }
}

/// GLFW internal API
//     Initialize WGL
//
GLFWbool _glfwInitWGL(void)
{
    _glfw.wgl.instance = LoadLibraryA("opengl32.dll");
    if (!_glfw.wgl.instance)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "WGL: Failed to load opengl32.dll");
        return GLFW_FALSE;
    }

    _glfw.wgl.CreateContext = (WGLCREATECONTEXT_T)
        GetProcAddress(_glfw.wgl.instance, "wglCreateContext");
    _glfw.wgl.DeleteContext = (WGLDELETECONTEXT_T)
        GetProcAddress(_glfw.wgl.instance, "wglDeleteContext");
    _glfw.wgl.GetProcAddress = (WGLGETPROCADDRESS_T)
        GetProcAddress(_glfw.wgl.instance, "wglGetProcAddress");
    _glfw.wgl.MakeCurrent = (WGLMAKECURRENT_T)
        GetProcAddress(_glfw.wgl.instance, "wglMakeCurrent");
    _glfw.wgl.ShareLists = (WGLSHARELISTS_T)
        GetProcAddress(_glfw.wgl.instance, "wglShareLists");

    return GLFW_TRUE;
}

// Terminate WGL
//
void _glfwTerminateWGL(void)
{ if (_glfw.wgl.instance)FreeLibrary(_glfw.wgl.instance);
}

// Create the OpenGL or OpenGL ES context
//
GLFWbool _glfwCreateContextWGL(_GLFWwindow* window,
                               const _GLFWctxconfig* ctxconfig,
                               const _GLFWfbconfig* fbconfig)
{
    int pixelFormat = 0;
    PIXELFORMATDESCRIPTOR pfd;
    HGLRC share = NULL;

    if (ctxconfig->client == GLFW_NO_API)return GLFW_TRUE;

    if (ctxconfig->share)share = ctxconfig->share->context.wgl.handle;

    window->context.wgl.dc = GetDC(window->win32.handle);
    if (!window->context.wgl.dc)
    { _glfwInputError(GLFW_PLATFORM_ERROR,"WGL: Failed to retrieve DC for window");
       return GLFW_FALSE;
    }
    if (!choosePixelFormat(window, fbconfig, &pixelFormat))return GLFW_FALSE;
    if (!DescribePixelFormat(window->context.wgl.dc,pixelFormat,sizeof(pfd),&pfd))
    { _glfwInputError(GLFW_PLATFORM_ERROR,
                        "WGL: Failed to retrieve PFD for selected pixel format");
      return GLFW_FALSE;
    }
    if (!SetPixelFormat(window->context.wgl.dc, pixelFormat, &pfd))
    { _glfwInputError(GLFW_PLATFORM_ERROR,"WGL: Failed to set selected pixel format");
      return GLFW_FALSE;
    }
    {   window->context.wgl.handle = wglCreateContext(window->context.wgl.dc);
        if (!window->context.wgl.handle)
        {
            _glfwInputError(GLFW_VERSION_UNAVAILABLE,
                            "WGL: Failed to create OpenGL context");
            return GLFW_FALSE;
        }
        if (share)
        {   if (!wglShareLists(share, window->context.wgl.handle))
            {  _glfwInputError(GLFW_PLATFORM_ERROR,
                                "WGL: Failed to enable sharing with specified OpenGL context");
                return GLFW_FALSE;
            }
        }
    }
    window->context.makeCurrent = makeContextCurrent;
    window->context.swapBuffers = swapBuffers;
    window->context.swapInterval = swapInterval;
    window->context.getProcAddress = getProcAddress;
    window->context.destroy = destroyContext;
    return GLFW_TRUE;
}
