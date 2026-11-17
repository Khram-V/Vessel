// GLFW 3.2 WGL - www.glfw.org

#ifndef _glfw3_wgl_context_h_
#define _glfw3_wgl_context_h_

typedef HGLRC (WINAPI * WGLCREATECONTEXT_T)(HDC);
typedef BOOL (WINAPI * WGLDELETECONTEXT_T)(HGLRC);
typedef PROC (WINAPI * WGLGETPROCADDRESS_T)(LPCSTR);
typedef BOOL (WINAPI * WGLMAKECURRENT_T)(HDC,HGLRC);
typedef BOOL (WINAPI * WGLSHARELISTS_T)(HGLRC,HGLRC);

// opengl32.dll function pointer typedefs
#define wglCreateContext _glfw.wgl.CreateContext
#define wglDeleteContext _glfw.wgl.DeleteContext
#define wglGetProcAddress _glfw.wgl.GetProcAddress
#define wglMakeCurrent _glfw.wgl.MakeCurrent
#define wglShareLists _glfw.wgl.ShareLists

#define _GLFW_RECREATION_NOT_NEEDED 0
#define _GLFW_RECREATION_REQUIRED   1
#define _GLFW_RECREATION_IMPOSSIBLE 2

#define _GLFW_PLATFORM_CONTEXT_STATE            _GLFWcontextWGL wgl
#define _GLFW_PLATFORM_LIBRARY_CONTEXT_STATE    _GLFWlibraryWGL wgl


// WGL-specific per-context data
//
typedef struct _GLFWcontextWGL
{   HDC       dc;
    HGLRC     handle;
    int       interval;
} _GLFWcontextWGL;

// WGL-specific global data
//
typedef struct _GLFWlibraryWGL
{   HINSTANCE                           instance;
    WGLCREATECONTEXT_T                  CreateContext;
    WGLDELETECONTEXT_T                  DeleteContext;
    WGLGETPROCADDRESS_T                 GetProcAddress;
    WGLMAKECURRENT_T                    MakeCurrent;
    WGLSHARELISTS_T                     ShareLists;
} _GLFWlibraryWGL;

GLFWbool _glfwInitWGL(void);
void _glfwTerminateWGL(void);
GLFWbool _glfwCreateContextWGL(_GLFWwindow* window,
                               const _GLFWctxconfig* ctxconfig,
                               const _GLFWfbconfig* fbconfig);
#endif // _glfw3_wgl_context_h_
