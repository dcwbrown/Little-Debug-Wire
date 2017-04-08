// DebugWIRE interface via LittleWire / Digispark device

#include "opendevice.h"  // From usb_dev_handle/software/example/library

#define VENDOR_ID   0x1781
#define PRODUCT_ID  0x0c9f
#define USB_TIMEOUT 5000

usb_dev_handle* dwOpenPort()
{
  usb_dev_handle *lw = NULL;
  usb_init();
  usbOpenDevice(&lw, VENDOR_ID, "*", PRODUCT_ID, "*", "*", NULL, NULL );
  return lw;
}

// dw_connect - returns baud rate if connected, or 0.

#define OutToLw  USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT
#define InFromLw USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN

int dw_connect(usb_dev_handle* lwHandle)
{
  uint16_t dwBuf[64];
  int      i;
  int      attemptcount;

  lwStatus = usb_control_msg(
    lwHandle, OutToLw, 60, 0, 0,  // Break and read timings
    0, 0, USB_TIMEOUT
  );
  if (lwStatus < 0) return 0;  // Connection failed.

  delay(100); // While debugWIRE is connecting any USB messages will fail, so give the
              // connection at least enough time to break for 100ms.

  attemptcount = 0;
  lwStatus = 0;
  while ((attemptcount < 10) && (lwStatus <= 0)) {
    attemptcount++;
    delay(50);
    lwStatus = usb_control_msg(
      lwHandle, InFromLw, 60, 0, 0,  // Read back timings
      (unsigned char*)dwBuf, sizeof(dwBuf), USB_TIMEOUT
    );
  }
  if (lwStatus < 18) {return 0;} // Failure - couldn't read any or enough pulse timings.


  // Average measurements and determine baud rate as pulse time in device cycles.

  uint32_t measurementCount = lwStatus / 2;
  uint32_t cyclesPerPulse = 0;
  for (i=measurementCount-9; i<measurementCount; i++) cyclesPerPulse += dwBuf[i];

  // Pulse cycle time for each measurement is <6*measurement + 8> cyclesPerPulse.
  cyclesPerPulse = (6*cyclesPerPulse) / 9  +  8;

  // Determine timing loop iteration counts for sending and receiving bytes

  dwBuf[0] = (cyclesPerPulse-8)/4;  // dwBitTime

  //  printf("Setting timing parameters:\n");
  //  printf("  dwBuf[0]  dwBitTime   %d\n", dwBuf[0]);

  // Send timing parameters to digispark

  lwStatus = usb_control_msg(
    lwHandle, OutToLw, 60, 1, 0,  // set uart loop counter
    (unsigned char*)dwBuf, 2, USB_TIMEOUT
  );
  if (lwStatus < 2) {return -1;} // Failed to set lopp counter

  return 16500000 / cyclesPerPulse;  // Return baud rate.
}


// dw_transfer - result is < 0 if failure,  length received if successful

int dw_transfer(usb_dev_handle* lwHandle, char *out, int outlen, char *in, int inlen) {
  int i;

  lwStatus = usb_control_msg(
    lwHandle, OutToLw, 60, 3, 0,  // send data and receive bytes
    out, outlen, USB_TIMEOUT
  );
  if (lwStatus < outlen) {return -1;} // Transfer failed

  attemptcount = 0;
  lwStatus = 0;
  while ((attemptcount < 10) && (lwStatus <= 0)) {
    attemptcount++;
    delay(50);
    lwStatus = usb_control_msg(
      lwHandle, InFromLw, 60, 0, 0,  // Read back dWIRE bytes
      (unsigned char*)in, inlen, USB_TIMEOUT
    );

    return lwStatus;
  }

  return 0;
}


char *littleWire_errorName (int lwStatus) {
  switch (lwStatus) {
  case  -1: return "I/O Error"; break;
  case  -2: return "Invalid paramenter"; break;
  case  -3: return "Access error"; break;
  case  -4: return "No device"; break;
  case  -5: return "Not found"; break;
  case  -6: return "Busy"; break;
  case  -7: return "Timeout"; break;
  case  -8: return "Overflow"; break;
  case  -9: return "Pipe"; break;
  case -10: return "Interrupted"; break;
  case -11: return "No memory"; break;
  case -12: return "Not supported"; break;
  case -99: return "Other"; break;
  default:  return "unknown";
  }
}


-----------------

unsigned char version;
int total_lwCount;
int i;


uint16_t widths[20];

int main(void)
{
	usb_dev_handle *lw = NULL;

	total_lwCount = littlewire_search();
	printf("Found %d Little Wire devices.\n", total_lwCount);
	if (total_lwCount < 1) exit(EXIT_FAILURE);

	lw = littlewire_connect_byID(0);

	if(lw == NULL){
		printf("> Could not connect to first Little Wire device.\n");
		exit(EXIT_FAILURE);
	}

	version = readFirmwareVersion(lw);
	printf("Little Wire firmware version: %d.%d\n",((version & 0xF0)>>4),(version&0x0F));

  int status = dw_connect(lw);

  if (status >= 0) {
    printf("Connected at %d baud.\n", status);

    // Try reading back the device type

    uint8_t command[1];  command[0] = 0xF3;  int commandlen = 1;

    uint16_t signature; signature = 0;

    status = dw_transfer(lw, command, commandlen, (char*)&signature, sizeof(signature));

    if (status < sizeof(signature)) {
      printf("Signature read failed.\n");
    } else {
      printf("Device signature: %04.4x\n", signature);
    }
  }

	return 0;
}


//  Control data is sent and received with the function usb_control_msg().
//
//  To request data from the device, use
//
//  int realNumBytes = usb_control_msg(
//          handle,             // handle obtained with usb_open()
//          USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, // bRequestType 0xC0
//          MY_REQUEST_ID,      // bRequest
//          value,              // wValue
//          index,              // wIndex
//          buffer,             // pointer to destination buffer
//          numBytesRequested,  // wLength
//          timeoutInMilliseconds
//      );
//
//  usb_control_msg() returns the number of bytes actually received from the
//  device. You may pass any information you like in the parameters value and
//  index to the device.
//
//
//  Sending a block of data to the device is similar:
//
//  int realNumBytes = usb_control_msg(
//          handle,             // handle obtained with usb_open()
//          USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, // bRequestType 0x40
//          MY_REQUEST_ID,      // bRequest
//          value,              // wValue
//          index,              // wIndex
//          buffer,             // pointer to buffer containing data
//          numBytesInBuffer,   // wLength
//          timeoutInMilliseconds
//      );
//
//
//
//
//  USB_TYPE_VENDOR   2<<5 = 0x40
//  USB_RECIP_DEVICE  0x00
//  USB_ENDPOINT_IN   0x80
//  USB_ENDPOINT_OUT  0x00
//
//
//  bRequestType for
//
//    In to pc:      0xC0
//    Out to device: 0x40