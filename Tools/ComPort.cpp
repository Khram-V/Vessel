//
//   ComPort.cpp: implementation of the ComPort class
//   Author: Jurgis Armanavichius
//   Based on original CSerialPort class by Remon Spekreijse (thank you)
//
#include "ComPort.h"

ComPort::ComPort() // Construction/Destruction
{    m_bOpened  = FALSE;
     m_Baud     = CBR_9600;
     m_DataBits = 8;
     m_Parity   = NOPARITY;
     m_StopBits = ONESTOPBIT;
}
ComPort::~ComPort(){ CloseCom(); }

BOOL ComPort::OpenCom( LPCTSTR lpPortName, DWORD baud, BYTE databits, BYTE parity, BYTE stopbits )
{   if( m_bOpened )return FALSE;
  BOOL fSuccess;   // we are already opened, so OpenCom failed
  DCB  dcb;
    m_Baud     = baud;
    m_DataBits = databits;
    m_Parity   = parity;
    m_StopBits = stopbits;
    m_hCommPort = CreateFile( lpPortName, GENERIC_READ|GENERIC_WRITE,
                0,      // exclusive access
                NULL,   // no security attrs
                OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL );
    if( m_hCommPort==INVALID_HANDLE_VALUE )return FALSE; // OpenCom failed
    GetCommState( m_hCommPort,&dcb );                    //  Setup new connection:
    dcb.BaudRate = m_Baud;
    dcb.ByteSize = m_DataBits;
    dcb.Parity   = m_Parity;
    dcb.StopBits = m_StopBits;
    dcb.fOutxDsrFlow = 0;   // we don't need handshake
    dcb.fDtrControl  = 0;
    dcb.fOutxCtsFlow = 0;
    dcb.fRtsControl  = 0;
    dcb.fBinary = TRUE;
    dcb.fParity = TRUE;
    fSuccess = SetCommState( m_hCommPort,&dcb );
    if( !fSuccess )return FALSE;    // Can't SetCommState -- OpenCom failed
    PurgeComm(m_hCommPort,PURGE_TXABORT | PURGE_RXABORT |
                          PURGE_TXCLEAR | PURGE_RXCLEAR );    // Clean up COM port
    return m_bOpened = TRUE;                                  // OpenCom OK
}
BOOL ComPort::CloseCom()
{  if( m_bOpened )
   {   m_bOpened=FALSE;
          PurgeComm( m_hCommPort,PURGE_TXABORT | PURGE_RXABORT
                           | PURGE_TXCLEAR | PURGE_RXCLEAR ); // Clean up COM port
          CloseHandle( m_hCommPort );
   }  return TRUE;
}
BOOL ComPort::WriteBlock( LPBYTE lpBuffer, DWORD dwBufferLength )
{ DWORD dwBytesWritten;
    if( m_bOpened )     // we are not connected
        if( WriteFile( m_hCommPort,lpBuffer,dwBufferLength,&dwBytesWritten,NULL ) )return TRUE;
        return FALSE;
}
DWORD ComPort::ReadBlock( LPBYTE lpBuffer,DWORD nMax )
{ COMSTAT ComStat;
  DWORD   dwErrorFlags,
          dwLength;
   if( !m_bOpened )return (DWORD)-1; // we are not connected
                                     // get number of bytes in queue
   if( ClearCommError( m_hCommPort,&dwErrorFlags,&ComStat ) )
        dwLength=min( (DWORD)nMax,ComStat.cbInQue );
   else dwLength=0;
   if( dwLength>0 )
   { if( !ReadFile( m_hCommPort,lpBuffer,dwLength,&dwLength,NULL ) )
     { dwLength=0; ClearCommError( m_hCommPort,&dwErrorFlags,&ComStat ) ;
     }
   } return dwLength;
}
#define SLEEP_TIME 5
BOOL ComPort::GetByte( LPBYTE rByte, DWORD dwMilliseconds )
{ BYTE  byt;
  DWORD pause_cnt;
  pause_cnt=dwMilliseconds/SLEEP_TIME; // we pause in SLEEP_TIME milliseconds increment
  while( 1 )
   { if( ReadBlock( &byt,1 )>0 ){ *rByte=byt; return TRUE; }
         if( !pause_cnt )break;
                 Sleep( SLEEP_TIME );
                 pause_cnt--;
   }     return FALSE;
}
