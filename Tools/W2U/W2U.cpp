От: https://www.cnblogs.com/ziwuge/archive/2011/11/05/2236968.html https://www.cnblogs.com/gakusei/articles/1585211.html
 Простое сопоставление, не проверено, не отвечает за результаты
 
 Функциональный прототип:
int MultiByteToWideChar( 
　　　　UINT CodePage, 
　　　　DWORD dwFlags, 
　　　　LPCSTR lpMultiByteStr, 
　　　　int cchMultiByte, 
　　　　LPWSTR lpWideCharStr, 
　　　　int cchWideChar 
　　); 
int WideCharToMultiByte(
　　　　UINT CodePage, 
　　　　DWORD dwFlags, 
　　　　LPWSTR lpWideCharStr, 
　　　　int cchWideChar, 
　　　　LPCSTR lpMultiByteStr, 
　　　　int cchMultiByte, 
　　　　LPCSTR lpDefaultChar, 
　　　　PBOOL pfUsedDefaultChar 
　　);
 
 
 В случае рассмотрения вопроса о безопасном использовании используются следующие общие шаги:
　　MultiByteToWideChar：
 1) Вызовите MultiByteToWideChar, передайте NULL для параметра lpWideCharStr, 0 для параметра cchWideChar и -1 для параметра cchMultiByte;
 2) Выделите память, достаточную для хранения преобразованной строки Юникода, ее размер - это значение, возвращаемое последним вызовом MultiByteToWideChar, умноженное на sizeof (wchar_t);
 3) Снова вызовите MultiByteToWideChar, на этот раз передав адрес буфера в качестве значения параметра lpWideCharStr и умножив возвращаемое значение первого вызова MultiByteToWideChar на sizeof (wchar_t) как значение параметра cchWideChar
 4) Используйте преобразованную строку символов;
 5) Освободить блок памяти, занятый строкой Unicode.
 
　　WideCharToMultiByte：
 Предпринятые шаги аналогичны предыдущим, с той лишь разницей, что возвращаемое значение - это непосредственно число байтов, необходимое для успешного преобразования, поэтому умножение не требуется.
 
 Во второй главе «Windows Core Programming» (символьная и строковая обработка) упоминается много символов и строк стандартных методов обработки, таких как проблемы строковых функций, в конце концов, следует ли использовать библиотеку C или использовать MS для реализации пояса суффикс _s.
 
 
　
 
 Параметры MultiByteToWideChar и WideCharToMultiByte подробно
 Следующая часть взята из: http://www.cnblogs.com/wanghao111/archive/2009/05/25/1489021.html#2270293
 
 WideCharToMultiByte Эта функция преобразует строку широких символов в указанную новую строку символов, такую ​​как ANSI, UTF8 и т. Д. Новая строка символов не обязательно должна быть многобайтовым набором символов.
 Параметры:
 CodePage: укажите кодовую страницу набора символов для преобразования. Это может быть любой набор символов, который был установлен или поставляется с системой. Вы также можете использовать одну из кодовых страниц, показанных ниже. 
           CP_ACP Текущая кодовая страница ANSI системы 
           CP_MACCP Текущая кодовая страница Macintosh системы 
           CP_OEMCP Текущая кодовая страница OEM-системы, код сканирования оборудования OEM 
           CP_SYMBOL Кодовая страница символов для Windows 2000 и более поздних версий, я не понимаю, что 
           CP_THREAD_ACP текущая кодовая страница ANSI потока для Windows 2000 и позже, я не понимаю, что 
           CP_UTF7 UTF-7, при установке этого значения значения lpDefaultChar и lpUsedDefaultChar должны быть равны NULL 
           CP_UTF8 UTF-8, при установке этого значения значения lpDefaultChar и lpUsedDefaultChar должны быть равны NULL  
           / * Я думаю, что чаще всего используются CP_ACP и CP_UTF8, первый преобразует широкие символы в ANSI, а второй преобразует в UTF8. * /
 
 dwFlags: укажите, как обращаться с символами, которые не были преобразованы, но функция будет работать быстрее без этого параметра, я установил его на 0. Настраиваемые значения показаны в следующей таблице: 
           WC_NO_BEST_FIT_CHARS преобразует символы Unicode, которые нельзя напрямую преобразовать в соответствующие многобайтовые символы, в символы по умолчанию, указанные в lpDefaultChar. Другими словами, если вы преобразуете Юникод в многобайтовые символы, а затем конвертируете обратно, вы не обязательно получите те же символы Юникода, потому что в течение этого периода могут использоваться символы по умолчанию. Эта опция может использоваться отдельно или вместе с другими опциями. 
           WC_COMPOSITECHECK преобразует составные символы в готовые символы. Его можно использовать в сочетании с любым из трех последних параметров, если он не объединен ни с одним из них, он совпадает с параметром WC_SEPCHARS. 
           WC_ERR_INVALID_CHARS Эта опция заставляет функцию не возвращаться при обнаружении недопустимых символов, а GetLastError возвращает код ошибки ERROR_NO_UNICODE_TRANSLATION. В противном случае функция автоматически отбрасывает недопустимые символы. Эта опция может использоваться только для UTF8. 
           WC_DISCARDNS отбрасывает символы, которые не занимают место во время преобразования, используется с WC_COMPOSITECHECK 
           WC_SEPCHARS создает отдельные символы во время преобразования. Это опция преобразования по умолчанию, которая используется с WC_COMPOSITECHECK 
     WC_DEFAULTCHAR использует символ по умолчанию вместо символа исключения при преобразовании (наиболее распространенным является ‘?’), И используется вместе с WC_COMPOSITECHECK. 
           / * Если указан WC_COMPOSITECHECK, функция преобразует синтезированные символы в готовые символы. Синтетические символы состоят из базового символа и символа, который не занимает места (например, фонетические символы европейских стран и китайский пиньинь), каждый из которых имеет различное значение символа. Готовые символы имеют однозначное значение, используемое для представления набора базовых символов и символов без пробелов. При указании параметра WC_COMPOSITECHECK вы также можете использовать последние 3 параметра, перечисленные в таблице выше, чтобы настроить правила преобразования для готовых символов. Эти параметры определяют поведение функции, когда она сталкивается с составным символом широких символов, который не имеет соответствующего готового символа. Они используются вместе с WC_COMPOSITECHECK. Если ни один из них не указан, по умолчанию используется функция WC_SEPCHARS. Для следующих кодовых страниц dwFlags должно быть 0, в противном случае функция возвращает код ошибки ERROR_INVALID_FLAGS. 50220 50221 50222 50225 50227 50229 52936 54936 57002–57011 65000 (UTF7) 42 (условное обозначение) 
 Для UTF8 dwFlags должен быть 0 или WC_ERR_INVALID_CHARS, в противном случае функция не сможет вернуться и установить код ошибки ERROR_INVALID_FLAGS, вы можете вызвать GetLastError, чтобы получить. * /
 
 lpWideCharStr: строка широких символов для преобразования. 
 cchWideChar: длина строки широких символов для преобразования, -1 означает преобразование в конец строки. 
 lpMultiByteStr: буфер для вывода новой строки после получения преобразования. 
 cbMultiByte: размер выходного буфера. Если он равен 0, lpMultiByteStr будет проигнорировано, и функция вернет требуемый размер буфера без использования lpMultiByteStr. 
 lpDefaultChar: указатель на символ. Используйте этот символ как символ по умолчанию, когда соответствующий символ не может быть найден в указанной кодировке. Если это NULL, используется системный символ по умолчанию. Для dwFlags, которые требуют, чтобы этот параметр был равен NULL, и использовать этот параметр, функция не сможет вернуть и установить код ошибки ERROR_INVALID_PARAMETER. 
 lpUsedDefaultChar: указатель для переключения переменной, чтобы указать, использовался ли символ по умолчанию. Для dwFlags, которые требуют, чтобы этот параметр был равен NULL, и использовать этот параметр, функция не сможет вернуть и установить код ошибки ERROR_INVALID_PARAMETER. lpDefaultChar и lpUsedDefaultChar установлены в NULL, функция будет быстрее. 
 / * Примечание. Неправильное использование функции WideCharToMultiByte повлияет на безопасность программы. Вызов этой функции может легко вызвать утечку памяти, потому что размер входного буфера, на который указывает lpWideCharStr, представляет собой большое количество символов, а размер выходного буфера, на который указывает lpMultiByteStr, представляет собой число байтов. Чтобы избежать утечек памяти, обязательно укажите подходящий размер выходного буфера. Мой метод состоит в том, чтобы вызвать WideCharToMultiByte один раз с cbMultiByte 0, чтобы получить требуемый размер буфера, выделить место для буфера, а затем снова вызвать WideCharToMultiByte, чтобы заполнить буфер. Подробнее см. Код ниже. Кроме того, преобразование из Unicode UTF16 в набор символов, отличный от Unicode, может привести к потере данных, поскольку набор символов не может найти символы, которые представляют определенные данные Unicode. * /
 
 Возвращаемое значение: если функция завершается успешно, а cbMultiByte не равно 0, возвращает количество байтов, записанных в lpMultiByteStr (включая ноль в конце строки); cbMultiByte равно 0, а затем возвращает количество байтов, необходимое для преобразования. Функция завершается ошибкой и возвращает 0.
 
 
 MultiByteToWideChar - это функция преобразования многобайтовых символов в широкие.
 Эта функция преобразует многобайтовую строку символов в строку широких символов (Unicode). Преобразуемая строка символов не обязательно является многобайтовой.
 Параметры, возвращаемое значение и меры предосторожности для этой функции см. В описании функции WideCharToMultiByte выше. Здесь кратко поясняются только dwFlags.
 
 
 
 dwFlags: укажите, следует ли преобразовывать в готовые символы или в синтезированные широкие символы, использовать ли пиктограммы для управляющих символов и как обращаться с недопустимыми символами. 
 MB_PRECOMPOSED всегда использует готовые символы, то есть, когда есть один готовый символ, декомпозированный базовый символ и свободный знак не будут использоваться. Это опция по умолчанию для функции и не может быть объединена с MB_COMPOSITE 
 MB_COMPOSITE Всегда использовать символы декомпозиции, то есть всегда использовать базовые символы + символы без пробелов 
 MB_ERR_INVALID_CHARS Установите эту опцию, функция завершится ошибкой с недопустимыми символами и вернет код ошибки ERROR_NO_UNICODE_TRANSLATION, в противном случае недопустимые символы будут отброшены 
 MB_USEGLYPHCHARS Используйте пиктографические символы вместо управляющих символов 
/ * Для следующих кодовых страниц dwFlags должно быть 0, в противном случае функция возвращает код ошибки ERROR_INVALID_FLAGS. 50220 50221 50222 50225 50227 50229 52936 54936 57002–57011 65000 (UTF7) 42 (условное обозначение). Для UTF8 dwFlags должен быть 0 или MB_ERR_INVALID_CHARS, в противном случае функция завершится ошибкой и вернет код ошибки ERROR_INVALID_FLAGS * /
 
 
 Также добавьте пример для справки, операционная среда (vc 6.0, 32-битная пиратская версия win7)
#include <windows.h>
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
     // TODO: Place code here.
    wchar_t wszTest[] = L"ziwuge";
         wchar_t wszTestNew [] = L "парк блогов ziwuge";
    int nwszTestLen = lstrlenW(wszTest);            // 6
    int nwszTestNewLen = lstrlenW(wszTestNew);        // 9
    int nwszTestSize = sizeof(wszTest);                // 14
    int nwszTestNewSize = sizeof(wszTestNew);        //    20
         int nChar = WideCharToMultiByte (CP_ACP, 0, wszTestNew, -1, NULL, 0, NULL, NULL); // 13, возвращенный результат уже содержит память, занятую '\ 0'
         nChar = nChar * sizeof (char); // 13, на самом деле, этот шаг не является необходимым, пожалуйста, смотрите предыдущее объяснение этой статьи
    char* szResult = new char[nChar];
    ZeroMemory(szResult, nChar);
    int i = WideCharToMultiByte(CP_ACP, 0, wszTestNew, -1, szResult, nChar, NULL, NULL);    // 13
    int nszResultLen = lstrlenA(szResult);            // 12
    int nszResultSize = sizeof(szResult);            // 4
 
    char szTest[] = "ziwuge";
         char szTestNew [] = "Парк блогов ziwuge";
    int nszTestLen = lstrlenA(szTest);                // 6
    int nszTestNewLen = lstrlenA(szTestNew);        // 12
    int nszTestSize = sizeof(szTest);                // 7
    int nszTestNewSize = sizeof(szTestNew);            // 13
         int nWChar = MultiByteToWideChar (CP_ACP, 0, szTestNew, -1, NULL, 0); // 10, возвращенный результат уже содержит память, занятую '\ 0'
    nWChar = nWChar * sizeof(wchar_t);                // 20
    wchar_t* wszResult = new wchar_t[nWChar];
    ZeroMemory(wszResult, nWChar);
    int j = MultiByteToWideChar(CP_ACP, 0, szTestNew, -1, wszResult, nWChar);    // 10
    int nwszResultLen = lstrlenW(wszResult);        // 9
    int nwszResultSize = sizeof(wszResult);            // 4
    return 0;
}
//=====================================
 Для поддержки кодировки Unicode требуется преобразование между многобайтовым и широким байтом. Эти две системные функции должны указывать кодовую страницу при их использовании.В реальном процессе приложения они сталкиваются с искаженными проблемами, а затем перечитывают «Программирование ядра Windows», чтобы обобщить правильное использование.
 Кодовая страница WideCharToMultiByte используется для маркировки кодовой страницы, связанной с вновь преобразованной строкой.
 Кодовая страница MultiByteToWideChar используется для маркировки кодовой страницы, связанной с многобайтовой строкой.
 Обычно используемые кодовые страницы CP_ACP и CP_UTF8.
 Использование кодовой страницы CP_ACP реализует преобразование между ANSI и Unicode.
 Используйте кодовую страницу CP_UTF8 для достижения преобразования между UTF-8 и Unicode.
 Вот реализация кода:
1.  ANSI to Unicode
wstring ANSIToUnicode( const string& str )
{
 int  len = 0;
 len = str.length();
 int  unicodeLen = ::MultiByteToWideChar( CP_ACP,
            0,
            str.c_str(),
            -1,
            NULL,
            0 );  
 wchar_t *  pUnicode;  
 pUnicode = new  wchar_t[unicodeLen+1];  
 memset(pUnicode,0,(unicodeLen+1)*sizeof(wchar_t));  
 ::MultiByteToWideChar( CP_ACP,
         0,
         str.c_str(),
         -1,
         (LPWSTR)pUnicode,
         unicodeLen );  
 wstring  rt;  
 rt = ( wchar_t* )pUnicode;
 delete  pUnicode; 
 
 return  rt;  
}
2.  Unicode to ANSI
string UnicodeToANSI( const wstring& str )
{
 char*     pElementText;
 int    iTextLen;
 // wide char to multi char
 iTextLen = WideCharToMultiByte( CP_ACP,
         0,
         str.c_str(),
         -1,
         NULL,
         0,
NULL,
         NULL );
 pElementText = new char[iTextLen + 1];
 memset( ( void* )pElementText, 0, sizeof( char ) * ( iTextLen + 1 ) );
 ::WideCharToMultiByte( CP_ACP,
         0,
         str.c_str(),
         -1,
         pElementText,
         iTextLen,
         NULL,
         NULL );
 string strText;
 strText = pElementText;
 delete[] pElementText;
 return strText;
}
3.  UTF-8 to Unicode
wstring UTF8ToUnicode( const string& str )
{
 int  len = 0;
 len = str.length();
 int  unicodeLen = ::MultiByteToWideChar( CP_UTF8,
            0,
            str.c_str(),
            -1,
            NULL,
            0 );  
 wchar_t *  pUnicode;  
 pUnicode = new  wchar_t[unicodeLen+1];  
 memset(pUnicode,0,(unicodeLen+1)*sizeof(wchar_t));  
 ::MultiByteToWideChar( CP_UTF8,
         0,
         str.c_str(),
         -1,
         (LPWSTR)pUnicode,
         unicodeLen );  
 wstring  rt;  
 rt = ( wchar_t* )pUnicode;
 delete  pUnicode; 
 
 return  rt;  
}
4.  Unicode to UTF-8    
string UnicodeToUTF8( const wstring& str )
{
 char*     pElementText;
 int    iTextLen;
 // wide char to multi char
 iTextLen = WideCharToMultiByte( CP_UTF8,
         0,
         str.c_str(),
         -1,
         NULL,
         0,
         NULL,
         NULL );
 pElementText = new char[iTextLen + 1];
 memset( ( void* )pElementText, 0, sizeof( char ) * ( iTextLen + 1 ) );
 ::WideCharToMultiByte( CP_UTF8,
         0,
         str.c_str(),
         -1,
         pElementText,
         iTextLen,
         NULL,
         NULL );
 string strText;
 strText = pElementText;
 delete[] pElementText;
 return strText;
}
