// 多字节编码转为UTF8编码  
bool MBToUTF8(vector<char>& pu8, const char* pmb, int32 mLen)  
{  
 // convert an MBCS string to widechar   
 int32 nLen = MultiByteToWideChar(CP_ACP, 0, pmb, mLen, NULL, 0);  
   
 WCHAR* lpszW = NULL;  
 try  
 {  
    lpszW = new WCHAR[nLen];  
 }  
 catch(bad_alloc &memExp)  
 {  
    return false;  
 }  
  
 int32 nRtn = MultiByteToWideChar(CP_ACP, 0, pmb, mLen, lpszW, nLen);  
   
 if(nRtn != nLen)  
 {  
    delete[] lpszW;  
    return false;  
 }  
 // convert an widechar string to utf8  
 int32 utf8Len = WideCharToMultiByte(CP_UTF8, 0, lpszW, nLen, NULL, 0, NULL, NULL);  
 if (utf8Len <= 0)  
 {  
     return false;  
 }  
 pu8.resize(utf8Len);  
 nRtn = WideCharToMultiByte(CP_UTF8, 0, lpszW, nLen, &*pu8.begin(), utf8Len, NULL, NULL);  
 delete[] lpszW;  
  
 if (nRtn != utf8Len)  
 {  
     pu8.clear();  
     return false;  
 }  
 return true;  
}  
  
// UTF8编码转为多字节编码  
bool UTF8ToMB(vector<char>& pmb, const char* pu8, int32 utf8Len)  
{  
    // convert an UTF8 string to widechar   
    int32 nLen = MultiByteToWideChar(CP_UTF8, 0, pu8, utf8Len, NULL, 0);  
  
    WCHAR* lpszW = NULL;  
    try  
    {  
        lpszW = new WCHAR[nLen];  
    }  
    catch(bad_alloc &memExp)  
    {  
        return false;  
    }  
  
    int32 nRtn = MultiByteToWideChar(CP_UTF8, 0, pu8, utf8Len, lpszW, nLen);  
  
    if(nRtn != nLen)  
    {  
        delete[] lpszW;  
        return false;  
    }  
  
    // convert an widechar string to Multibyte   
    int32 MBLen = WideCharToMultiByte(CP_ACP, 0, lpszW, nLen, NULL, 0, NULL, NULL);  
    if (MBLen <=0)  
    {  
        return false;  
    }  
    pmb.resize(MBLen);  
    nRtn = WideCharToMultiByte(CP_ACP, 0, lpszW, nLen, &*pmb.begin(), MBLen, NULL, NULL);  
    delete[] lpszW;  
  
    if(nRtn != MBLen)  
    {  
        pmb.clear();  
        return false;  
    }  
    return true;  
}  
  
// 多字节编码转为Unicode编码  
bool MBToUnicode(vector<wchar_t>& pun, const char* pmb, int32 mLen)  
{  
    // convert an MBCS string to widechar   
    int32 uLen = MultiByteToWideChar(CP_ACP, 0, pmb, mLen, NULL, 0);  
  
    if (uLen<=0)  
    {  
        return false;  
    }  
    pun.resize(uLen);  
  
    int32 nRtn = MultiByteToWideChar(CP_ACP, 0, pmb, mLen, &*pun.begin(), uLen);  
  
    if (nRtn != uLen)  
    {  
        pun.clear();  
        return false;  
    }  
    return true;  
}  
  
//Unicode编码转为多字节编码  
bool UnicodeToMB(vector<char>& pmb, const wchar_t* pun, int32 uLen)  
{  
    // convert an widechar string to Multibyte   
    int32 MBLen = WideCharToMultiByte(CP_ACP, 0, pun, uLen, NULL, 0, NULL, NULL);  
    if (MBLen <=0)  
    {  
        return false;  
    }  
    pmb.resize(MBLen);  
    int nRtn = WideCharToMultiByte(CP_ACP, 0, pun, uLen, &*pmb.begin(), MBLen, NULL, NULL);  
  
    if(nRtn != MBLen)  
    {  
        pmb.clear();  
        return false;  
    }  
    return true;  
}  
  
// UTF8编码转为Unicode  
bool UTF8ToUnicode(vector<wchar_t>& pun, const char* pu8, int32 utf8Len)  
{  
    // convert an UTF8 string to widechar   
    int32 nLen = MultiByteToWideChar(CP_UTF8, 0, pu8, utf8Len, NULL, 0);  
    if (nLen <=0)  
    {  
        return false;  
    }  
    pun.resize(nLen);  
    int32 nRtn = MultiByteToWideChar(CP_UTF8, 0, pu8, utf8Len, &*pun.begin(), nLen);  
  
    if(nRtn != nLen)  
    {  
        pun.clear();  
        return false;  
    }  
  
    return true;  
}  
  
// Unicode编码转为UTF8  
bool UnicodeToUTF8(vector<char>& pu8, const wchar_t* pun, int32 uLen)  
{  
    // convert an widechar string to utf8  
    int32 utf8Len = WideCharToMultiByte(CP_UTF8, 0, pun, uLen, NULL, 0, NULL, NULL);  
    if (utf8Len<=0)  
    {  
        return false;  
    }  
    pu8.resize(utf8Len);  
    int32 nRtn = WideCharToMultiByte(CP_UTF8, 0, pun, uLen, &*pu8.begin(), utf8Len, NULL, NULL);  
  
    if (nRtn != utf8Len)  
    {  
        pu8.clear();  
        return false;  
    }  
    return true;  
}  
