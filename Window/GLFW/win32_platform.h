
// GLFW 3.2 Win32 - www.glfw.org

#ifndef _glfw3_win32_platform_h_
#define _glfw3_win32_platform_h_

// This is a workaround for the fact that glfw3.h needs to export APIENTRY (for
// example to allow applications to correctly declare a GL_ARB_debug_output
// callback) but windows.h assumes no one will define APIENTRY before it does
#undef APIENTRY

// GLFW on Windows is Unicode only and does not work in MBCS mode
#ifndef UNICODE
 #define UNICODE
#endif

// GLFW requires Windows XP or later
#if WINVER < 0x0501
 #undef WINVER
 #define WINVER 0x0501
#endif
#if _WIN32_WINNT < 0x0501
 #undef _WIN32_WINNT
 #define _WIN32_WINNT 0x0501
#endif

// GLFW uses DirectInput8 interfaces
#define DIRECTINPUT_VERSION 0x0800

#include <wctype.h>
#include <windows.h>
#include <dbt.h>

#if defined(_MSC_VER)
 #include <malloc.h>
 #define strdup _strdup
#endif

// HACK: Define macros that some windows.h variants don't
#ifndef WM_MOUSEHWHEEL
 #define WM_MOUSEHWHEEL 0x020E
#endif
#ifndef WM_DWMCOMPOSITIONCHANGED
 #define WM_DWMCOMPOSITIONCHANGED 0x031E
#endif
#ifndef WM_COPYGLOBALDATA
 #define WM_COPYGLOBALDATA 0x0049
#endif
#ifndef WM_UNICHAR
 #define WM_UNICHAR 0x0109
#endif
#ifndef UNICODE_NOCHAR
 #define UNICODE_NOCHAR 0xFFFF
#endif
#ifndef WM_DPICHANGED
 #define WM_DPICHANGED 0x02E0
#endif
#ifndef GET_XBUTTON_WPARAM
 #define GET_XBUTTON_WPARAM(w) (HIWORD(w))
#endif
#ifndef EDS_ROTATEDMODE
 #define EDS_ROTATEDMODE 0x00000004
#endif
#ifndef DISPLAY_DEVICE_ACTIVE
 #define DISPLAY_DEVICE_ACTIVE 0x00000001
#endif

#if WINVER < 0x0601
typedef struct tagCHANGEFILTERSTRUCT
{ DWORD cbSize;
  DWORD ExtStatus;
} CHANGEFILTERSTRUCT, *PCHANGEFILTERSTRUCT;
#ifndef MSGFLT_ALLOW
 #define MSGFLT_ALLOW 1
#endif
#endif /*Windows 7*/

// winmm.dll function pointer typedefs
typedef DWORD (WINAPI * TIMEGETTIME_T)(void);
#define _glfw_timeGetTime _glfw.win32.winmm.timeGetTime

// user32.dll function pointer typedefs
typedef BOOL (WINAPI * SETPROCESSDPIAWARE_T)(void);
typedef BOOL (WINAPI * CHANGEWINDOWMESSAGEFILTEREX_T)(HWND,UINT,DWORD,PCHANGEFILTERSTRUCT);
#define _glfw_SetProcessDPIAware _glfw.win32.user32.SetProcessDPIAware
#define _glfw_ChangeWindowMessageFilterEx _glfw.win32.user32.ChangeWindowMessageFilterEx

// dwmapi.dll function pointer typedefs
typedef HRESULT (WINAPI * DWMISCOMPOSITIONENABLED_T)(BOOL*);
typedef HRESULT (WINAPI * DWMFLUSH_T)(VOID);
#define _glfw_DwmIsCompositionEnabled _glfw.win32.dwmapi.DwmIsCompositionEnabled
#define _glfw_DwmFlush _glfw.win32.dwmapi.DwmFlush

#include "wgl_context.h"

#define _GLFW_WNDCLASSNAME L"GLFW30"

#define _glfw_dlopen(name) LoadLibraryA(name)
#define _glfw_dlclose(handle) FreeLibrary((HMODULE) handle)
#define _glfw_dlsym(handle, name) GetProcAddress((HMODULE) handle, name)

#define _GLFW_EGL_NATIVE_WINDOW  ((EGLNativeWindowType) window->win32.handle)
#define _GLFW_EGL_NATIVE_DISPLAY EGL_DEFAULT_DISPLAY

#define _GLFW_PLATFORM_WINDOW_STATE         _GLFWwindowWin32  win32
#define _GLFW_PLATFORM_LIBRARY_WINDOW_STATE _GLFWlibraryWin32 win32
#define _GLFW_PLATFORM_LIBRARY_TIME_STATE   _GLFWtimeWin32    win32_time
#define _GLFW_PLATFORM_LIBRARY_TLS_STATE    _GLFWtlsWin32     win32_tls
#define _GLFW_PLATFORM_MONITOR_STATE        _GLFWmonitorWin32 win32
#define _GLFW_PLATFORM_CURSOR_STATE         _GLFWcursorWin32  win32


// Win32-specific per-window data
//
typedef struct _GLFWwindowWin32
{
    HWND                handle;
    HICON               bigIcon;
    HICON               smallIcon;

    GLFWbool            cursorTracked;
    GLFWbool            iconified;

    // The last received cursor position, regardless of source
    int                 lastCursorPosX, lastCursorPosY;

} _GLFWwindowWin32;


// Win32-specific global data
//
typedef struct _GLFWlibraryWin32
{
    HWND                helperWindowHandle;
    DWORD               foregroundLockTimeout;
    char*               clipboardString;
    char                keyName[64];
    short int           publicKeys[512];
    short int           nativeKeys[GLFW_KEY_LAST + 1];
    // Where to place the cursor when re-enabled
    double              restoreCursorPosX, restoreCursorPosY;
    // The window whose disabled cursor mode is active
    _GLFWwindow*        disabledCursorWindow;

    struct {
        HINSTANCE       instance;
        TIMEGETTIME_T   timeGetTime;
    } winmm;

    struct {
        HINSTANCE                     instance;
        SETPROCESSDPIAWARE_T          SetProcessDPIAware;
        CHANGEWINDOWMESSAGEFILTEREX_T ChangeWindowMessageFilterEx;
    } user32;
    struct {
        HINSTANCE       instance;
        DWMISCOMPOSITIONENABLED_T DwmIsCompositionEnabled;
        DWMFLUSH_T      DwmFlush;
    } dwmapi;

} _GLFWlibraryWin32;


// Win32-specific per-monitor data
//
typedef struct _GLFWmonitorWin32
{
    // This size matches the static size of DISPLAY_DEVICE.DeviceName
    WCHAR               adapterName[32];
    WCHAR               displayName[32];
    char                publicAdapterName[64];
    char                publicDisplayName[64];
    GLFWbool            modesPruned;
    GLFWbool            modeChanged;

} _GLFWmonitorWin32;


// Win32-specific per-cursor data
//
typedef struct _GLFWcursorWin32
{
    HCURSOR handle;

} _GLFWcursorWin32;


// Win32-specific global timer data
//
typedef struct _GLFWtimeWin32
{
    GLFWbool            hasPC;
    uint64_t            frequency;

} _GLFWtimeWin32;


// Win32-specific global TLS data
//
typedef struct _GLFWtlsWin32
{
    GLFWbool        allocated;
    DWORD           context;

} _GLFWtlsWin32;


GLFWbool _glfwRegisterWindowClassWin32(void);
void _glfwUnregisterWindowClassWin32(void);

GLFWbool _glfwInitThreadLocalStorageWin32(void);
void _glfwTerminateThreadLocalStorageWin32(void);

WCHAR* _glfwCreateWideStringFromUTF8Win32(const char* source);
char* _glfwCreateUTF8FromWideStringWin32(const WCHAR* source);

void _glfwInitTimerWin32(void);

GLFWbool _glfwSetVideoModeWin32(_GLFWmonitor* monitor, const GLFWvidmode* desired);
void _glfwRestoreVideoModeWin32(_GLFWmonitor* monitor);

#endif // _glfw3_win32_platform_h_
