
// GLFW 3.5 Win32 - www.glfw.org

#include "internal.h"
#if defined(GLFW_BUILD_WIN32_THREAD)

/// GLFW platform API

GLFWbool _glfwPlatformCreateTls(_GLFWtls* tls)
{  tls->win32.index = TlsAlloc();
   if (tls->win32.index == TLS_OUT_OF_INDEXES)
   { _glfwInputError(GLFW_PLATFORM_ERROR,"Win32: Failed to allocate TLS index");
     return GLFW_FALSE;
   }
   tls->win32.allocated = GLFW_TRUE;
   return GLFW_TRUE;
}
void _glfwPlatformDestroyTls(_GLFWtls* tls)
{  if (tls->win32.allocated)TlsFree(tls->win32.index);
   memset(tls, 0, sizeof(_GLFWtls));
}
void* _glfwPlatformGetTls(_GLFWtls* tls)
{  return TlsGetValue(tls->win32.index);
}
void _glfwPlatformSetTls(_GLFWtls* tls, void* value)
{  TlsSetValue(tls->win32.index, value);
}
GLFWbool _glfwPlatformCreateMutex(_GLFWmutex* mutex)
{  InitializeCriticalSection(&mutex->win32.section);
   return mutex->win32.allocated = GLFW_TRUE;
}
void _glfwPlatformDestroyMutex(_GLFWmutex* mutex)
{  if (mutex->win32.allocated)DeleteCriticalSection(&mutex->win32.section);
   memset(mutex, 0, sizeof(_GLFWmutex));
}
void _glfwPlatformLockMutex(_GLFWmutex* mutex)
{  EnterCriticalSection(&mutex->win32.section);
}
void _glfwPlatformUnlockMutex(_GLFWmutex* mutex)
{  LeaveCriticalSection(&mutex->win32.section);
}
#endif // GLFW_BUILD_WIN32_THREAD

