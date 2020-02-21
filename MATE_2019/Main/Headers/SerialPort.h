#ifndef SERIALPORT_H
#define SERIALPORT_H

#define MAX_DATA_LENGTH 255

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

class SerialPort
{
 private:
  HANDLE handler;
  bool connected;
  COMSTAT status;
  DWORD errors;

 public:
  SerialPort();
  ~SerialPort();

  void openSerialPort(const char *portName, const int baudRate);
  void closeSerialPort();
  int readSerialPort(char *buffer, unsigned int buf_size);
  bool writeSerialPort(char *buffer, unsigned int buf_size);
  bool isConnected();
};

#endif  // SERIALPORT_H
