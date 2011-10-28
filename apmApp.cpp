#include "apmApp.h"

using namespace std;

int main (int argc, char *argv []) {
  char aux [20];
  char *device = aux;
  UCHAR mode = 0x40; //FIFO mode
  int bytesToRead;
  int num;
  FtdiDevice dev;

  if (argc > 1) {
    device = argv[1];
    cout << "Device serial number: " << device << endl;
  }
  else {
    dev.print_device_info_list ();
    cout << "Choose a device by its serial number: ";
    cin >> device;
  }

  dev.open_device (device);
  dev.set_mode (mode);
  bytesToRead = dev.get_num_bytes ();

  if (bytesToRead == 0) {
    cout << "Nothing to read from the receive queue." << endl;
    return 0;
  }

  while (bytesToRead != 0) {
    char buffer [bytesToRead];
    num = dev.read_data (buffer, bytesToRead);
    cout << num << endl;
    bytesToRead = dev.get_num_bytes ();
  }


  //  dev.print_data_from_file ();
  return 0;
}
