
//Алиса, почему-то твоя программа не запускается, проверь пожалуйста. В то же время все предложенные тобой функции прекрасно отрабатывают в моей большой программуле.
//Давайте проверим и оптимизируем код, чтобы он гарантированно запускался как отдельное приложение:

#define _WIN32_IE 0x0300

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h> //commctrl.h
//#include <richedit.h>
//#include <tchar.h>    // Для поддержки Unicode
//#include <wchar.h>

/* Определяем макросы для Unicode
#ifdef UNICODE
#define STATUSCLASSNAME STATUSCLASSNAMEW
#else
#define STATUSCLASSNAME STATUSCLASSNAMEA
#endif */

// Константы для размеров
#define MENU_HEIGHT 40
#define STATUS_HEIGHT 30
#define FONT_SIZE 14

// Идентификаторы меню
#define ID_FILE_OPEN 100
#define ID_FILE_SAVE 101
#define ID_FILE_EXIT 102
#define ID_HELP_ABOUT 200

// Глобальные переменные
HMENU hMenu = NULL;
HWND hStatusWnd = NULL;
HFONT hFont = NULL;
RECT windowRect = {0};
RECT clientRect = {0};

// Функция обновления текста в статусной строке
void SetStatusText( HWND hwndStatus, LPCWSTR text )
   { SendMessageW( hwndStatus,SB_SETTEXTW,0,(LPARAM)text ); }

/* Функция для проверки положения курсора
BOOL IsCursorInTransparentArea(HWND hwnd, POINT pt)
{ //RECT clientRect;
    GetClientRect(hwnd, &clientRect); // Проверяем, находится ли точка в запрещённых зонах
    if (pt.y < MENU_HEIGHT ||
        pt.y > clientRect.bottom - STATUS_HEIGHT) return FALSE;

    // Проверяем, находится ли точка в клиентской области
    if (pt.x>=0 && pt.x<clientRect.right &&
        pt.y>=MENU_HEIGHT && pt.y<clientRect.bottom-STATUS_HEIGHT)return TRUE;
    return FALSE;
}
*/
static
LRESULT CALLBACK WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{ POINT pt;
  static RECT statusRect;
  static BOOL bActive = FALSE;
    switch (msg)
    {
        case WM_CREATE:
            // Настройка пропуска кликов мыши
//            SetWindowLongPtr(hwnd, GWL_EXSTYLE,
//                (LONG_PTR)(GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_TRANSPARENT | WS_EX_LAYERED));

            // Сохраняем начальные размеры окна
//            if( !GetWindowRect(hwnd,&windowRect) )return 0;
//            if( !GetClientRect(hwnd,&clientRect) )return 0;
            GetWindowRect(hwnd,&windowRect);
            GetClientRect(hwnd,&clientRect);
//            SetActiveWindow(hwnd);
//            SetFocus(hwnd);

            break;
        case WM_PAINT:
        { PAINTSTRUCT ps; HDC hdc=BeginPaint(hwnd,&ps);
            if( hdc )
            { // Рисование меню
              RECT rectMenu={ 0, 0, clientRect.right, MENU_HEIGHT };
              FillRect( hdc,&rectMenu,(HBRUSH)(COLOR_MENU+1) );
              // Рисование статусной строки
              RECT rectStatus={ 0,clientRect.bottom-STATUS_HEIGHT,
                                  clientRect.right,clientRect.bottom };
              FillRect(hdc, &rectStatus, (HBRUSH)(COLOR_WINDOW + 1));
            }
            EndPaint(hwnd, &ps);
/*              SetFocus(hwnd);
                // Принудительно активируем окно
                SetForegroundWindow(hwnd);
                SetActiveWindow(hwnd); */
        }   break;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_FILE_OPEN:
                  MessageBoxW(hwnd,L"Открыт файл",L"Информация",MB_OK); break;
                case ID_FILE_SAVE:
                  MessageBoxW(hwnd,L"Сохранен файл",L"Информация",MB_OK); break;
                case ID_HELP_ABOUT:
                  MessageBoxW(hwnd,L"О программе",L"Информация",MB_OK); break;
                case ID_FILE_EXIT: DestroyWindow(hwnd); break;
            } break;

        case WM_SIZE:
            GetClientRect(hwnd, &clientRect);
            MoveWindow(hStatusWnd,
                0,
                clientRect.bottom - STATUS_HEIGHT,
                clientRect.right,
                STATUS_HEIGHT,
                TRUE);

            int parts[] = {clientRect.right};
            SendMessage(hStatusWnd, SB_SETPARTS, 1, (LPARAM)parts);
            break;

#if 0
       case WM_NCHITTEST:
            // Получаем позицию курсора
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            ScreenToClient(hwnd, &pt);

printf( "NcTest x=%d,y=%d \n",pt.x,pt.y );

            // Сначала проверяем заголовок и неклиентскую область
            LRESULT result = DefWindowProc(hwnd, msg, wParam, lParam);

            if (result == HTCAPTION || result == HTLEFT || result == HTRIGHT ||
                result == HTTOP || result == HTTOPLEFT || result == HTTOPRIGHT ||
                result == HTBOTTOM || result == HTBOTTOMLEFT || result == HTBOTTOMRIGHT) {
                return result;  // Возвращаем стандартное поведение для заголовка и рамок
            }

            // Проверяем, находится ли курсор в области статусной строки
            GetWindowRect(hStatusWnd, &statusRect);
            MapWindowPoints(hStatusWnd, hwnd, (POINT*)&statusRect, 2);

            if (PtInRect(&statusRect, pt)) {
                return HTCLIENT;  // Отключаем прозрачность для статусной строки
            }

            // Проверяем, находится ли курсор в клиентской области
            if (PtInRect(&clientRect, pt)) {
                return HTTRANSPARENT;  // Сохраняем прозрачность для клиентской области
            }
            // Для остальных областей возвращаем стандартное поведение
            return DefWindowProc(hwnd, msg, wParam, lParam);

        case WM_MOUSEACTIVATE:
            // Важно: возвращаем MA_ACTIVATE для активации окна
            bActive = TRUE;
            return (LRESULT)MA_ACTIVATE;
            // Возвращаем MA_ACTIVATE_ANDEATTAINCE для принудительной активации
//            return (LRESULT)MA_ACTIVATE_ANDEATTAINCE;

        case WM_ACTIVATE:
            // Обработка активации
            if (wParam & WA_ACTIVE || wParam & WA_CLICKACTIVE) {
                SetFocus(hwnd);
                // Принудительно активируем окно
                SetForegroundWindow(hwnd);
                SetActiveWindow(hwnd);
            }
            break;
        case WM_SETFOCUS:
            // Обработка установки фокуса
            SetActiveWindow(hwnd);
            break;

        case WM_KILLFOCUS:
            // Обработка потери фокуса
            // Можно попробовать вернуть фокус
            SetFocus(hwnd);
            break;
//#if 0
        case WM_ACTIVATE:
            // Обработка активации
            if (wParam & WA_ACTIVE || wParam & WA_CLICKACTIVE) {
                bActive=TRUE;
                SetFocus(hwnd);
//              SetActiveWindow(hwnd);
            } else bActive=FALSE;
            break;

        case WM_SIZE:                 // Обработка изменения размеров окна
            clientRect.right = LOWORD(lParam);
            clientRect.bottom = HIWORD(lParam);
            if (hStatusWnd)SendMessage(hStatusWnd,WM_SIZE,0,0);
                                         // Обновление позиции статусной строки
            { MoveWindow( hStatusWnd,0,clientRect.bottom - STATUS_HEIGHT,
                                       clientRect.right, STATUS_HEIGHT, TRUE);
         //   Обновляем части статусной строки
              int L[]={clientRect.right};
              SendMessageW( hStatusWnd,SB_SETPARTS,1,(LPARAM)L);
            } break;

        case WM_NCHITTEST:
        { POINT pt;
            pt.x = GET_X_LPARAM(lParam); // LOWORD(lParam); //
            pt.y = GET_Y_LPARAM(lParam); // HIWORD(lParam); //
            ScreenToClient(hwnd, &pt);
            if (IsCursorInTransparentArea(hwnd, pt))return HTTRANSPARENT;
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        case WM_GETMINMAXINFO:
            ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 400;
            ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 300;
            break;
        case WM_WINDOWPOSCHANGING:     // Обработка WM_WINDOWPOSCHANGING
        { WINDOWPOS* wpos = (WINDOWPOS*)lParam;
          if (wpos->flags & SWP_NOSIZE)
            return DefWindowProc(hwnd, msg, wParam, lParam);
          clientRect.right = wpos->cx;
          clientRect.bottom = wpos->cy;
          break;
        }
        case WM_ERASEBKGND:
            return TRUE; // Отключаем стандартную очистку фона
        case WM_CTLCOLORSTATIC:
        {   SetTextColor((HDC)wParam, RGB(0, 0, 0));
            SetBkMode((HDC)wParam, TRANSPARENT);
            return (LRESULT)GetStockObject(WHITE_BRUSH);
        }
       case WM_NCHITTEST:
            // Получаем позицию курсора
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
//          ScreenToClient(hwnd, &pt);
          LRESULT result = DefWindowProc( hwnd,msg,wParam,lParam );
printf( "NcTest %d == %s \n",result,result==HTCLIENT?"client":" ??? " );
            // Сначала проверяем заголовок и неклиентскую область
//            if (result == HTCAPTION || result == HTLEFT || result == HTRIGHT ||
//                result == HTTOP || result == HTTOPLEFT || result == HTTOPRIGHT ||
//                result == HTBOTTOM || result == HTBOTTOMLEFT || result == HTBOTTOMRIGHT)
//                return result;  // Возвращаем стандартное поведение для заголовка и рамок

          if( result!=HTCLIENT )return 0; //DefWindowProc( NULL,msg,wParam,lParam );
          return result;
/*      case 96:
        case WM_TIMER:
           SetWindowLongPtr(hwnd, GWL_EXSTYLE,
               (LONG_PTR)( GetWindowLongPtr(hwnd, GWL_EXSTYLE)) & ~(WS_EX_TRANSPARENT|WS_EX_LAYERED));
           return DefWindowProc(hwnd, msg, wParam, lParam);
*/
#endif
        case WM_LBUTTONDOWN:
//         DefWindowProc(NULL, msg, wParam, lParam);
//         SendMessageW( NULL,msg,wParam,lParam ); //( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
           SetWindowLongPtr( hwnd,GWL_EXSTYLE,
           (LONG_PTR)(GetWindowLongPtr(hwnd,GWL_EXSTYLE) | WS_EX_TRANSPARENT | WS_EX_LAYERED));
           break; //return DefWindowProc( NULL,msg,wParam,lParam );
        case WM_MOUSEMOVE:
        { //POINT pt;
/*          if (bActive)SetFocus(hwnd);
            // Добавляем обработку для активации окна
            if (IsWindowVisible(hwnd))
            {   SetFocus(hwnd);
                SetActiveWindow(hwnd);
            } */
            pt.x = LOWORD(lParam); //GET_X_LPARAM(lParam);
            pt.y = HIWORD(lParam); // GET_Y_LPARAM(lParam);
            ScreenToClient(hwnd,&pt);
printf( "mouse x=%d,y=%d \n",pt.x,pt.y );

            if (hStatusWnd)
            { wchar_t buffer[100];
              int L[]={clientRect.right};
              SendMessageW( hStatusWnd,SB_SETPARTS,1,(LPARAM)L);
              swprintf( buffer, L"X: %d, Y: %d", pt.x,pt.y );
              //    wsprintfW(buffer, L"X: %d, Y: %d", pt.x, pt.y);
              SetStatusText( hStatusWnd,buffer );
            }
            break;
        }
        case WM_DESTROY:        // Освобождение всех ресурсов
            if (hStatusWnd) DestroyWindow(hStatusWnd);
            if (hMenu) DestroyMenu(hMenu);
            if (hFont) DeleteObject(hFont);
            PostQuitMessage(0); return 0;
        default: return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Регистрация класса окна
    WNDCLASSEXW wc={ sizeof(WNDCLASSEXW) };
    ZeroMemory( &wc,sizeof( WNDCLASSEXW ) );  //hInstance=GetModuleHandle( NULL );
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW; // | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = L"WG01"; //L"TransparentWindowClass";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    // Инициализация Common Controls
  INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(icex);
    icex.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icex);

    if (!RegisterClassExW( &wc )) {
        MessageBoxW(NULL, L"Ошибка регистрации класса окна", L"Ошибка", MB_OK);
        return 0;
    }
    // Создание окна
    HWND hwnd = CreateWindowExW
    (   WS_EX_LAYERED, // | WS_EX_TRANSPARENT, // | WS_EX_APPWINDOW,
        wc.lpszClassName,                  // L"TransparentWindowClass",
        L"1_TransparentWindow",            // L"Прозрачное окно",
        WS_OVERLAPPEDWINDOW
        | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUPWINDOW | WS_VISIBLE | WS_POPUPWINDOW | WS_POPUP,
        CW_USEDEFAULT,CW_USEDEFAULT,       // 100, 100,
        800, 600,
        HWND_DESKTOP,                       // дескриптор родительского окна
        NULL, hInstance, NULL
    );
if (!hwnd)
{ DWORD error = GetLastError();                // Получаем код ошибки
    LPWSTR buffer;
    FormatMessageW
    ( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                         (LPWSTR)&buffer, 0, NULL );

    // Выводим сообщение об ошибке
    MessageBoxW(NULL, buffer, L"Ошибка создания окна", MB_OK | MB_ICONERROR);
    LocalFree(buffer);
    return 12;
}
//  Установка прозрачности
    SetLayeredWindowAttributes( hwnd,0,180,LWA_ALPHA );

// Отображение окна
    ShowWindow( hwnd,nCmdShow );
    UpdateWindow(hwnd);

// Создание шрифта
    LOGFONTW lf = { 0 };
    lf.lfHeight = -MulDiv(FONT_SIZE, GetDeviceCaps(GetDC(hwnd), LOGPIXELSY), 72);
    lstrcpyW(lf.lfFaceName, L"Arial");
    lf.lfWeight = FW_NORMAL;
    lf.lfCharSet = DEFAULT_CHARSET;
    hFont = CreateFontIndirectW(&lf);
    if (!hFont) {
        MessageBoxW(NULL, L"Ошибка создания шрифта", L"Ошибка", MB_OK);
        return 0;
    }
    SendMessageW(hwnd, WM_SETFONT, (WPARAM)hFont, TRUE);

#if 1
    // Создание меню
    hMenu = CreateMenu();
    if (!hMenu){ MessageBoxW(NULL, L"Ошибка создания меню", L"Ошибка", MB_OK); return 31; }

    HMENU hFileMenu = CreatePopupMenu();
    if (!hFileMenu){ MessageBoxW(NULL, L"Ошибка создания подменю", L"Ошибка", MB_OK); return 32; }
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_OPEN, L"Открыть");
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_SAVE, L"Сохранить");
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_EXIT, L"Выход");
    AppendMenuW(hMenu, MF_POPUP,(UINT_PTR)hFileMenu, L"Файл");

    HMENU hHelpMenu = CreatePopupMenu();
    if (!hHelpMenu){ MessageBoxW(NULL,L"Ошибка создания подменю", L"Ошибка",MB_OK); return 34; }
    AppendMenuW(hHelpMenu, MF_STRING, ID_HELP_ABOUT, L"О программе");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hHelpMenu, L"Справка");

    SetMenu( hwnd,hMenu );

// Отображение окна
   ShowWindow( hwnd,nCmdShow );
   UpdateWindow(hwnd);

// Создание статусной строки
int wID=12;
#if 0
hStatusWnd = CreateWindowExW(
    0,                       // расширенные стили
    STATUSCLASSNAMEW,        // класс окна статусной строки
    NULL,                    // заголовок окна (не используется)
    WS_CHILD | WS_VISIBLE | WS_BORDER | SBARS_SIZEGRIP | CCS_BOTTOM, // стили окна
    0,                       // x-координата
    clientRect.bottom - STATUS_HEIGHT, // y-координата
    clientRect.right,        // ширина
    STATUS_HEIGHT,           // высота
    hwnd,                    // родительское окно
    (HMENU)wID, // NULL,     // идентификатор меню (не используется)
    hInstance,               // экземпляр приложения
    NULL                     // дополнительные параметры создания
);
#else
InitCommonControls();
hStatusWnd = CreateStatusWindowW(
        WS_CHILD | WS_VISIBLE,
        wc.lpszClassName, //имя класса вашего окна
        hwnd,  //родитель, его получаем при вызове hwnd= CreateWindow();
        wID ); //уникальный индикатор любое ваше число!!
#endif
if (!hStatusWnd) // Получаем код ошибки
{   DWORD error=GetLastError(); LPWSTR buffer;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
       NULL, error,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPWSTR)&buffer,0,NULL );
    // Выводим сообщение об ошибке
    MessageBoxW(NULL, buffer, L"Ошибка создания статусной строки", MB_OK | MB_ICONERROR);
    LocalFree(buffer);
    return 15;
} else
{ //RECT clientRect;
    // Получаем актуальные размеры клиентской области
    GetClientRect(hwnd,&clientRect);
    // Устанавливаем части статусной строки
    int L[]={clientRect.right};
    SendMessageW( hStatusWnd,SB_SETPARTS,1,(LPARAM)L);
    // Устанавливаем начальный текст
    SetStatusText( hStatusWnd,L"Готов к работе" );
}

printf( "5 Status 5\n" );


#endif
  // Цикл обработки сообщений
/*struct
{ HWND	 hwnd;
  UINT	 message;
  WPARAM wParam;
  LPARAM lParam;
  DWORD  time;
  POINT  pt; } MSG; */
//#include <stdIO.h>

MSG msg;
// Отображение окна
    ShowWindow( hwnd,nCmdShow );
    UpdateWindow(hwnd);

// Цикл обработки сообщений

while (GetMessage(&msg, NULL,0,0))
//while ( GetMessage( &msg,hwnd,0,0 ) )    //,PM_REMOVE))
{
printf( "Msg=%d\n",msg.message ); //if( msg.message==799 || msg.message==49299 || msg.message==160 )continue;

    TranslateMessage(&msg),DispatchMessage(&msg);

//      if( msg.message==96 || msg.message==WM_TIMER )
//        SetWindowLongPtr( hwnd,GWL_EXSTYLE,
//        (LONG_PTR)((GetWindowLongPtr(hwnd,GWL_EXSTYLE)) & (~(WS_EX_TRANSPARENT|WS_EX_LAYERED))));
        if( msg.message==96 )
          SetWindowLongPtr( hwnd,GWL_EXSTYLE,
          (LONG_PTR)((GetWindowLongPtr(hwnd,GWL_EXSTYLE)) & ~WS_EX_TRANSPARENT));
/*
    if( TranslateMessage(&msg) )DispatchMessage(&msg); //else continue;
    if( !TranslateMessage(&msg) )        // Обработка ошибки перевода сообщения
      { MessageBoxW(NULL,L"Ошибка перевода сообщения",L"Ошибка",MB_OK); break; }
    if( !DispatchMessage(&msg) ) // Обработка ошибки диспетчеризации сообщения
      { MessageBoxW(NULL,L"Ошибка диспетчеризации сообщения",L"Ошибка",MB_OK); break; }
*/
}
// Дополнительная проверка на корректное завершение
if (msg.message == WM_QUIT) { // Освобождение ресурсов перед выходом
    // Нормальное завершение
    // Освобождение ресурсов
    if (hStatusWnd) DestroyWindow(hStatusWnd);
    if (hMenu) DestroyMenu(hMenu);
    if (hFont) DeleteObject(hFont);
    return msg.wParam;
} else { // Аварийное завершение
    MessageBoxW(NULL, L"Аварийное завершение программы", L"Ошибка", MB_OK);
    return -11;
}   return msg.wParam;
}
