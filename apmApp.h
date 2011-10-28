#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include "ftd2xx.h"

#define MAX_USB_BUFFER  0x10000 // 64KB
using namespace std;

class FtdiDevice {
  FT_STATUS ftstatus;
  FT_HANDLE ftHandle;
  ofstream binFile;
  char *snum;
 public:
  FtdiDevice ();
  ~FtdiDevice ();
  int print_device_info_list ();
  void open_device (char *serial);
  int close_device ();
  int set_mode (UCHAR mode);
  int print_mode ();
  int get_num_bytes ();
  int read_data (char *rxBuffer, DWORD rxBytesInQueue);
  int print_data_from_file ();
};

FtdiDevice::FtdiDevice () {
  binFile.open ("data.raw", ios::binary);
  if (!binFile) {
    cerr << "Error while opening file \"data.raw\"." << endl;
    exit (1);
  }
  cout << "Binary file opened (write mode) successfully!" << endl;
}

FtdiDevice::~FtdiDevice () {
  close_device ();
  if (binFile.is_open()) {
    binFile.close();
    cout << "Binary file closed." << endl;
  }
}


// Functions definitions
int FtdiDevice::print_device_info_list() {
  DWORD numdevs;
  ftstatus = FT_CreateDeviceInfoList(&numdevs);
  if (ftstatus != FT_OK) {
    cerr << "Could not retreive device info list.\n";
    exit (1);
  }

  cout << "Number of devices: " << numdevs << endl;
  // Get device info list
  FT_DEVICE_LIST_INFO_NODE *deviceList;
  try {
    deviceList = new FT_DEVICE_LIST_INFO_NODE [numdevs];
    ftstatus = FT_GetDeviceInfoList(deviceList, &numdevs);
    if (ftstatus == FT_OK) {
      for (unsigned int i = 0; i < numdevs; i++) {
	cout << "======= Device: " << i << " =======" << endl;
	cout << "Flags: " << deviceList[i].Flags << endl;
	cout << "Type: " << deviceList[i].Type << endl;
	cout << "ID: " << deviceList[i].ID << endl;
	cout << "Location ID: " << deviceList[i].LocId << endl;
	cout << "Serial Number: " << deviceList[i].SerialNumber << endl;
	cout << "Description: " << deviceList[i].Description << endl;
	cout << "ftHandle: " << deviceList[i].ftHandle << endl;
      }
    }
    else {
      cerr << "Couldn't get device info.\n";
      exit (1);
    }
  }
  catch (...) {
    cerr << "Something wrong happened.\n";
    delete [] deviceList;
    exit (1);
  }
  delete [] deviceList;
  return 0;
}

void FtdiDevice::open_device (char *serial) {
  cout << serial << endl;
  snum = serial;
  ftstatus = FT_OpenEx(snum, FT_OPEN_BY_SERIAL_NUMBER, &ftHandle);
  if (ftstatus == FT_OK) {
    cout << "Device with serial number: " << snum << " was opened successfully.\n";
  }
  else {
    cerr << "Device with serial number: " << snum << " couldn't be opened.\n";
    exit (1);
  }
}

int FtdiDevice::close_device () {
  ftstatus = FT_Close(ftHandle);
  if (ftstatus != FT_OK) {
    cerr << "Device with serial number: " << snum << " couldn't be closed.\n";
    exit (1);
  }

  cout << "Device with serial number: " << snum << " was closed successfully.\n";
  return 0;
}

int FtdiDevice::set_mode (UCHAR mode) {
  UCHAR mask = 0xff;
  UCHAR reset_mode = 0x00;
  // Reset device
  ftstatus = FT_SetBitMode(ftHandle, mask, reset_mode);
  sleep(1); // wait 10 ms
  ftstatus = FT_SetBitMode(ftHandle, mask, mode);
  if (ftstatus != FT_OK) {
    cout << "Couldn't set bit mode: " << mode << endl;
    return 1;
  }
  printf ("Set bit mode is: %X \n", mode);
  UCHAR latencyTimer = 2;
  FT_SetLatencyTimer(ftHandle, latencyTimer);
  FT_SetUSBParameters(ftHandle, MAX_USB_BUFFER, MAX_USB_BUFFER);
  FT_SetFlowControl(ftHandle, FT_FLOW_RTS_CTS, 0, 0);
  if (ftstatus != FT_OK) {
    cerr << "Error(s) setting device flow control.\n";
    exit (1);
  }
  return 0;
}

int FtdiDevice::print_mode () {
  UCHAR mode;
  ftstatus = FT_GetBitMode(ftHandle, &mode);
  if (ftstatus == FT_OK) {
    printf ("Bit mode is: %X \n", mode);
    return 0;
  }
  else {
    cerr << "Couldn't get bit mode." << endl;
    exit (1);
  }
}

int FtdiDevice::get_num_bytes () {
  DWORD rxBytesInQueue;
  DWORD txBytesInQueue;
  DWORD event;

  FT_GetStatus (ftHandle, &rxBytesInQueue, &txBytesInQueue, &event);
  cout << "Number of characters in receive queue: " << rxBytesInQueue << endl;
  return rxBytesInQueue;
}

int FtdiDevice::read_data (char * rxBuffer, DWORD rxBytesInQueue) {
  DWORD bytesReturned;

  ftstatus = FT_Read (ftHandle, rxBuffer, rxBytesInQueue, &bytesReturned);
  if (ftstatus !=  FT_OK) {
    cerr << "Read failed.\n";
    exit (1);
  }

  cout << "Data read successful.\n";

  for (unsigned int i = 0; i < rxBytesInQueue; i++)
    binFile << rxBuffer [i];

  binFile << endl;

  return bytesReturned;
}

int FtdiDevice::print_data_from_file () {
  if (binFile.is_open ()) {
    binFile.close();
    cout << "Binary file is closed." << endl;
  }

  ifstream readBinFile ("data.raw", ios::binary);

  if (!readBinFile) {
    cerr << "Error while opening file \"data.raw\"." << endl;
    exit (1);
  }

  cout << "Binary file opened (read mode) successfully!" << endl;

  long byte;

  while (readBinFile >> byte)
    cout << byte << endl;


  readBinFile.close();
  cout << "Binary file closed." << endl;
  return 0;
}
