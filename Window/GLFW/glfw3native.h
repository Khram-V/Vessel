
// * GLFW 3.2 - www.glfw.org
// * A library for OpenGL, window and input

#ifndef _glfw3_native_h_
#define _glfw3_native_h_
#ifdef __cplusplus
extern "C" {
#endif

#if defined(GLFW_EXPOSE_NATIVE_WIN32)
 #undef APIENTRY
 #include <windows.h>
#endif

#if defined(GLFW_EXPOSE_NATIVE_WIN32)
GLFWAPI const char* glfwGetWin32Adapter(GLFWmonitor* monitor);
GLFWAPI const char* glfwGetWin32Monitor(GLFWmonitor* monitor);
GLFWAPI HWND glfwGetWin32Window(GLFWwindow* window);
#endif

#if defined(GLFW_EXPOSE_NATIVE_WGL)
GLFWAPI HGLRC glfwGetWGLContext(GLFWwindow* window);
#endif

#ifdef __cplusplus
}
#endif
#endif /* _glfw3_native_h_ */

