//
// ComPort.h: interface for the ComPort class.
//
#if !defined(_COMPORT_H_)
#define _COMPORT_H_
#include <windows.h>
class ComPort
{ HANDLE m_hCommPort;
  BOOL   m_bOpened;
  DWORD  m_Baud; BYTE m_DataBits,m_Parity,m_StopBits;
public: ComPort();
  virtual ~ComPort();
     BOOL OpenCom( LPCTSTR lpPortName="COM1", DWORD baud = CBR_9600,
                                              BYTE databits = 8,
                                              BYTE parity   = NOPARITY,
                                              BYTE stopbits = ONESTOPBIT );
     BOOL  CloseCom();
     BOOL  GetByte( LPBYTE rByte, DWORD dwMilliseconds );
     DWORD ReadBlock( LPBYTE lpBuffer,DWORD nMax );
     BOOL  WriteBlock( LPBYTE lpBuffer,DWORD dwBufferLength );
     BOOL  PutByte( BYTE wByte ){ return WriteBlock(&wByte,1); }
     BOOL  GetOpenedStatus(){ return m_bOpened; }
};
#endif // !defined(_COMPORT_H_)
