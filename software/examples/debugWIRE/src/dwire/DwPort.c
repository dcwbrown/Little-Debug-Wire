/// DebugPort.c

// DebugWire output command flag bits:
//
//     00000001    1     Send break
//     00000010    2     Set timing parameter
//     00000100    4     Send bytes
//     00001000    8     Wait for start bit
//     00010000   16     Read bytes
//     00100000   32     Read pulse widths
//
// Supported combinations
//    33 - Send break and read pulse widths
//     2 - Set timing parameters
//     4 - Send bytes
//    20 - Send bytes and read response (normal command)
//    28 - Send bytes, wait and read response (e.g. after programming, run to BP)
//    36 - Send bytes and receive 0x55 pulse widths
//
// Note that the wait for start bit loop also monitors the dwState wait for start
// bit flag, and is arranged so that sending a 33 (send break and read pulse widths)
// will abort a pending wait.


#include "littleWire.h"

#define VENDOR_ID   0x1781
#define PRODUCT_ID  0x0c9f
#define USB_TIMEOUT 5000
#define OUT_TO_LW   USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT
#define IN_FROM_LW  USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN


void PortFail(char *msg) {usb_close(Port); Port = 0; Fail(msg);}


void SetDwireBaud() {
  int status;
  int tries;
  int i;
  uint16_t times[64];

  tries = 0;  status = 0;
  while ((tries < 50)  &&  (status <= 0)) {
    tries++;
    delay(20);
    // Read back timings
    status = usb_control_msg(Port, IN_FROM_LW, 60, 0, 0, (char*)times, sizeof(times), USB_TIMEOUT);
  }
  if (status < 18) {PortFail("Digispark couldn't read back 0x55 break response from AVR");}

  // Average measurements and determine baud rate as pulse time in device cycles.

  uint32_t measurementCount = status / 2;
  uint32_t cyclesPerPulse = 0;
  for (i=measurementCount-9; i<measurementCount; i++) cyclesPerPulse += times[i];

  // Pulse cycle time for each measurement is 6*measurement + 8 cyclesPerPulse.
  cyclesPerPulse = (6*cyclesPerPulse) / 9  +  8;

  // Determine timing loop iteration counts for sending and receiving bytes

  times[0] = (cyclesPerPulse-8)/4;  // dwBitTime

  // Send timing parameters to digispark
  status = usb_control_msg(Port, OUT_TO_LW, 60, 2, 0, (char*)times, 2, USB_TIMEOUT);
  if (status < 0) {PortFail("Failed to set debugWIRE port baud rate");}

  Ws("Connected at "); Wd(16500000 / cyclesPerPulse, 1); Wsl(" baud.");
}




void BreakAndSync() {
  int status;

  // Break and read timings
  status = usb_control_msg(Port, OUT_TO_LW, 60, 33, 0, 0, 0, USB_TIMEOUT);
  if (status < 0) {PortFail("Digispark couldn't get debugWIRE break response from AVR");}

  delay(100); // Don't send USB meassages while digispark is trying to interrupt the AVR
  SetDwireBaud();
}




void ConnectPort() {
  usb_init();
  usbOpenDevice(&Port, VENDOR_ID, "*", PRODUCT_ID, "*", "*", NULL, NULL );
  if (!Port) {Fail("Couldn't connect to digispark.");}
  BreakAndSync();
}






void DwTransfer(const u8 *out, int outlen, u8* in, int inlen) {
  int status;
  int tries;
  u8  outState;

  AssertMessage(outlen > 0, "DwTransfer requires at least 1 byte to send to device");

  if (Port == 0) {ConnectPort();}

  // send data and receive bytes
  outState = (inlen == 0) ? 0x04 : 0x14;
  tries = 0;
  status = usb_control_msg(Port, OUT_TO_LW, 60, outState, 0, (char*)out, outlen, USB_TIMEOUT);
  while ((tries < 50) && (status <= 0)) {
    // Wait for previous operation to complete
    tries++;
    delay(20);
    status = usb_control_msg(Port, OUT_TO_LW, 60, outState, 0, (char*)out, outlen, USB_TIMEOUT);
  }
  if (status < outlen) {Ws("Failed to send bytes to AVR, status "); Wd(status,1); PortFail("");}

  if (inlen > 0) {
    tries = 0;
    status = 0;
    while ((tries < 50) && (status <= 0)) {
      tries++;
      delay(20);
      // Read back dWIRE bytes
      status = usb_control_msg(Port, IN_FROM_LW, 60, 0, 0, (char*)in, inlen, USB_TIMEOUT);
    }
    if (status > inlen) {Wsl("More bytes read than expected, ignoreing extra bytes.");}
    if (status < inlen) {
      Ws("Failed to read response after sending <");
      Whexbuf(out, outlen);
      Ws(">, status "); Wd(status,1);
      if (status > 0) {Ws(", received: <"); Whexbuf(in, status); PortFail(">");
      } else {PortFail(", received nothing");}
    }
  }
}


void DwTransferSync(const u8 *out, int outlen) {
  int status;

  // send data and receive pulse widths
  status = usb_control_msg(Port, OUT_TO_LW, 60, 36, 0, (char*)out, outlen, USB_TIMEOUT);
  if (status < outlen) {Ws("Failed to send command and receive 0x55 response from AVR, status ="); Wd(status,1); PortFail(".");}
  SetDwireBaud();
}




#define DwWrite      "Don't use DwWrite, use DwTransfer"
#define DwSerialRead "Don't use DwSerialRead, use DwTransfer"




u8 hi(int w) {return (w>>8)&0xff;}
u8 lo(int w) {return (w   )&0xff;}


int DwTransferReadByte(u8 *out, int outlen) {
  u8 byte = 0;
  DwTransfer(out, outlen, &byte, 1);
  return byte;
}

int DwTransferReadWord(u8 *out, int outlen) {
  u8 buf[2] = {0};
  DwTransfer(out, outlen, buf, 2);
  return (buf[0] << 8) | buf[1];
}




void CheckDevice() {if (DeviceType<0) {Fail("Device not recognised.");}}
int  IoregSize()   {CheckDevice(); return Characteristics[DeviceType].ioregSize;}
int  SramSize()    {CheckDevice(); return Characteristics[DeviceType].sramSize;}
int  EepromSize()  {CheckDevice(); return Characteristics[DeviceType].eepromSize;}
int  FlashSize()   {CheckDevice(); return Characteristics[DeviceType].flashSize;}   // In bytes
int  PageSize()    {CheckDevice(); return Characteristics[DeviceType].pageSize;}    // In bytes
int  DWDRreg()     {CheckDevice(); return Characteristics[DeviceType].DWDR;}
int  DWDRaddr()    {CheckDevice(); return Characteristics[DeviceType].DWDR + 0x20;} // IO regs come after the 32 regs r0-r31
int  DataLimit()   {CheckDevice(); return 32 + IoregSize() + SramSize();}

void SetSizes(int signature) {
  int i=0; while (Characteristics[i].signature  &&  Characteristics[i].signature != signature) {i++;}

  if (Characteristics[i].signature) {
    DeviceType = i;
    Ws("Device recognised as "); Wsl(Characteristics[DeviceType].name);
  } else {
    DeviceType = -1;
    Ws("Unrecognised device signature: "); Wx(signature,4); Fail("");
  }
}




void DwReadRegisters(u8 *registers, int first, int count) {
  //wsl("Read Registers.");
  // Read Registers (destroys PC and BP)
  DwTransfer(ByteArrayLiteral(
    0x66,                 // Access registers/memory mode
    0xD0, 0, first,       // Set PC = first
    0xD1, 0, first+count, // Set BP = limit
    0xC2, 1, 0x20),       // Start register read
    registers, count
  );
}


void DwWriteRegisters(u8 *registers, int first, int count) {
  //wsl("Write Registers.");
  // Write Registers (destroys PC and BP)
  DwTransfer(ByteArrayLiteral(
    0x66,                 // Access registers/memory mode
    0xD0, 0, first,       // Set PC = first
    0xD1, 0, first+count, // Set BP = limit
    0xC2, 5, 0x20),       // Start register write
    0,0
  );
  DwTransfer(registers, count, 0,0);
}


void DwUnsafeReadAddr(int addr, int len, u8 *buf) {
  // Do not read addresses 30, 31 or DWDR as these interfere with the read process
  DwTransfer(ByteArrayLiteral(
    0x66, 0xD0, 0,0x1e, 0xD1, 0,0x20,       // Set PC=0x001E, BP=0x0020 (i.e. address register Z)
    0xC2, 5, 0x20, lo(addr), hi(addr),      // Write first SRAM address to Z
    0xD0, 0,0, 0xD1, hi(len*2), lo(len*2),  // Set PC=0, BP=2*length
    0xC2, 0, 0x20),                         // Start the read
    buf, len
  );
}

void DwReadAddr(int addr, int len, u8 *buf) {
  // Read range before r30
  int len1 = min(len, 30-addr);
  if (len1 > 0) {DwUnsafeReadAddr(addr, len1, buf); addr+=len1; len-=len1; buf+=len1;}

  // Registers 30 and 31 are cached - use the cached values.
  if (addr == 30  &&  len > 0) {buf[0] = R30; addr++; len--; buf++;}
  if (addr == 31  &&  len > 0) {buf[0] = R31; addr++; len--; buf++;}

  // Read range from 32 to DWDR
  int len2 = min(len, DWDRaddr()-addr);
  if (len2 > 0) {DwUnsafeReadAddr(addr, len2, buf); addr+=len2; len-=len2; buf+=len2;}

  // Provide dummy 0 value for DWDR
  if (addr == DWDRaddr()  &&  len > 0) {buf[0] = 0; addr++; len--; buf++;}

  // Read anything beyond DWDR no more than 128 bytes at a time
  while (len > 128) {DwUnsafeReadAddr(addr, 128, buf); addr+=128; len -=128; buf+=128;}
  if (len > 0) {DwUnsafeReadAddr(addr, len, buf);}
}


void DwUnsafeWriteAddr(int addr, int len, const u8 *buf) {
  // Do not write addresses 30, 31 or DWDR as these interfere with the write process
  DwTransfer(ByteArrayLiteral(
    0x66, 0xD0, 0,0x1e, 0xD1, 0,0x20,           // Set PC=0x001E, BP=0x0020 (i.e. address register Z)
    0xC2, 5, 0x20, lo(addr), hi(addr),          // Write SRAM address of first IO register to Z
    0xD0, 0,1, 0xD1, hi(len*2+1), lo(len*2+1),  // Set PC=0, BP=2*length
    0xC2, 4, 0x20),                             // Start the write
    0,0
  );
  DwTransfer(buf, len, 0,0);
}

void DwWriteAddr(int addr, int len, const u8 *buf) {
  // Write range before r30
  int len1 = min(len, 30-addr);
  if (len1 > 0) {DwUnsafeWriteAddr(addr, len1, buf); addr+=len1; len-=len1; buf+=len1;}

  // Registers 30 and 31 are cached - update the cached values.
  if (addr == 30  &&  len > 0) {R30 = buf[0]; addr++; len--; buf++;}
  if (addr == 31  &&  len > 0) {R31 = buf[0]; addr++; len--; buf++;}

  // Write range from 32 to DWDR
  int len2 = min(len, DWDRaddr()-addr);
  if (len2 > 0) {DwUnsafeWriteAddr(addr, len2, buf); addr+=len2; len-=len2; buf+=len2;}

  // (Ignore anything for DWDR - as the DebugWIRE port it is in use and wouldn't retain a value anyway)
  if (addr == DWDRaddr()  &&  len > 0) {addr++; len--; buf++;}

  // Write anything beyond DWDR
  if (len > 0) {DwUnsafeWriteAddr(addr, len, buf);}
}


void DwReadFlash(int addr, int len, u8 *buf) {
  int limit = addr + len;
  if (limit > FlashSize()) {Fail("Attempt to read beyond end of flash.");}
  while (addr < limit) {
    int length = min(limit-addr, 128); // Read no more than 128 bytes at a time.
    //Ws("ReadFlashBlock at $"); Wx(addr,1); Ws(", length $"); Wx(length,1); Wl();
    DwTransfer(ByteArrayLiteral(
      0x66, 0xD0, 0,0x1e, 0xD1, 0,0x20,           // Set PC=0x001E, BP=0x0020 (i.e. address register Z)
      0xC2, 5, 0x20, lo(addr),hi(addr),           // Write addr to Z
      0xD0, 0,0, 0xD1, hi(2*length),lo(2*length), // Set PC=0, BP=2*len
      0xC2, 2, 0x20),                             // Read length bytes from flash starting at first
      buf, length
    );
    addr += length;
    buf  += length;
  }
}


void DwReconnect() {
  PC = 2 * (DwTransferReadWord(ByteArrayLiteral(0xF0)) - 1 );
  u8 buf[2];
  DwReadRegisters(buf, 30, 2);
  R30 = buf[0];
  R31 = buf[1];
}

void DwConnect() {
  DwReconnect();
  SetSizes(DwTransferReadWord(ByteArrayLiteral(0xF3)));
}

void DwReset() {
  //SerialBreak(SerialPort, BreakLength);
  DwTransferSync(ByteArrayLiteral(0x07));
  DwReconnect();
}

void DwDisable() {
  DwTransfer(ByteArrayLiteral(0x06), 0,0);
}


void DwTrace() { // Execute one instruction
  int p = PC/2;
  DwTransferSync(ByteArrayLiteral(
    0x66,                      // Register or memory access mode
    0xD0, 0, 30,               // Set up to set registers starting from r30
    0xD1, 0, 32,               // Register limit is 32 (i.e. stop at r31)
    0xC2, 5, 0x20,             // Select reigster write mode and start
    R30, R31,                  // Cached value of r30 and r31
    0x60, 0xD0, hi(p), lo(p),  // Address to restart execution at
    0x31                       // Continue execution (single step)
  ));
  DwReconnect();
}


void DwGo() { // Begin executing.
  u8 context = TimerEnable ? 0x40 : 0x60;
  int p = PC/2;
  int b = BP/2;

  DwTransfer(ByteArrayLiteral(
    0x66,                      // Register or memory access mode
    0xD0, 0, 30,               // Set up to set registers starting from r30
    0xD1, 0, 32,               // Register limit is 32 (i.e. stop at r31)
    0xC2, 5, 0x20,             // Select reigster write mode and start
    R30, R31),                 // Cached value of r30 and r31
    0,0
  );

  if (BP >= 0) {
    DwTransfer(ByteArrayLiteral(
      0xD1, hi(b), lo(b)),     // Set breakpoint for execution to stop at
      0,0
    );
    context |= 0x01;
  }

  DwTransfer(ByteArrayLiteral(
    context,
    0xD0, hi(p), lo(p),        // Address to restart execution at
    0x30),                     // Continue execution (go)
    0,0
  );
}


/*

/// DebugWire protocol notes

See RikusW's excellent work at http://www.ruemohr.org/docs/debugwire.html.


DebugWire command byte interpretation:

06      00 xx x1 10   Disable dW (Enable ISP programming)
07      00 xx x1 11   Reset

20      00 10 00 00   go start reading/writing SRAM/Flash based on low byte of IR
21      00 10 00 01   go read/write a single register
23      00 10 00 11   execute IR (single word instruction loaded with D2)

30      00 11 00 00   go normal execution
31      00 11 00 01   single step (Rikusw says PC increments twice?)
32      00 11 00 10   go using loaded instruction
33      00 11 00 11   single step using slow loaded instruction (specifically spm)
                      will generate break and 0x55 output when complete.

t: disable timers
40/60   01 t0 00 00   Set GO context  (No bp?)
41/61   01 t0 00 01   Set run to cursor context (Run to hardware BP?)
43/63   01 t0 00 11   Set step out context (Run to return instruction?)
44/64   01 t0 01 00   Set up for single step using loaded instruction
46/66   01 t0 01 10   Set up for read/write using repeating simulated instructions
59/79   01 t1 10 01   Set step-in / autostep context or when resuming a sw bp (Execute a single instruction?)
5A/7A   01 t1 10 10   Set single step context



83      10 d0 xx dd   Clock div

C2      11 00 00 10   Set read/write mode (folowed by 0/4 SRAM, 1/5 regs, 2 flash)

w:  word operation (low byte only if 0)
cc: control regs: 0: PC, 1: BP, 2: IR, 3: Sig.
Cx/Dx   11 0w xx cc   Set control reg  (Cx for byte register, Dx for word register)
Ex/Fx   11 1w xx cc   Read control reg (Ex for byte register, Fx for word register)


Modes:

SRAM repeating instructions:
C2 00                   C2 04
ld  r16,Z+       or     in r16,DWDR
out DWDR,r16     or     st Z+,r16

Regs repeating instructions
C2 01            or     C2 05
out DWDR,r0      or     in r0,DWDR
out DWDR,r1      or     in r1,DWDR
out DWDR,r2      or     in r2,DWDR
...                     ....

Flash repeating instructions
C2 03
lpm r?,Z+        or    ?unused
out SWDR,r?      or    ?unused



-------------------------------------------------------------------------



40/60   0   0   0   0   0   GO                         Set GO context  (No bp?)
41/61   0   0   0   0   1   Run to cursor              Set run to cursor context (Run to hardware BP?)
43/63   0   0   0   1   1   Step out                   Set step out context (Run to return instruction?)
44/64   0   0   1   0   0   Write flash page           Set up for single step using loaded instruction
46/66   0   0   1   1   0   Use virtual instructions   Set up for read/write using repeating simulated instructions
59/79   1   1   0   0   1   Step in/autostep           Set step-in / autostep context or when resuming a sw bp (Execute a single instruction?)
5A/7A   1   1   0   1   0   Single step                Set single step context
        ^   ^   ^   ^   ^
        |   |   |   '---'------ 00 no break, 01 Break at BP, 10 ?, 11 ? break at return?
        |   |   '-------------- PC represents Flash (0) or virtual space (1)
        '---'------------------




20      0 0 0   go start reading/writing SRAM/Flash based on low byte of PC
21      0 0 1   single step read/write a single register
23      0 1 1   single step an instruction loaded with D2
30      1 0 0   go normal execution
31      1 0 1   single step (Rikusw says PC increments twice?)
32      1 1 0   go using loaded instruction
33      1 1 1   single step using slow loaded instruction (specifically spm)
                      will generate break and 0x55 output when complete.
        ^ ^ ^
        | | '---- Single step - stop after 1 instruction
        | '------ Execute (at least initially) instruction loaded with D2.
        '-------- Use Flash (1) or virtual memory selected by C2 (0)


Resume execution:              60/61/79/7A 30
Resume from SW BP:             79 32
Step out:                      63 30
Execute instruction (via D2):  ?? 23
Read/write registers/SRAM:     66 20
Write single register:         66 21



Resuming execution

D0 00 00 xx -- set PC, xx = 40/60 - 41/61 - 59/79 - 5A/7A
D1 00 01 -- set breakpoint (single step in this case)
D0 00 00 30 -- set PC and GO







Writing a Flash Page

66
D0 00 1A D1 00 20 C2 05 20 03 01 05 40 00 00 -- Set X, Y, Z
D0 1F 00                                     -- Set PC to 0x1F00, inside the boot section to enable spm--

64
D2  01 CF  23        -- movw r24,r30
D2  BF A7  23        -- out SPMCSR,r26 = 03 = PGERS
D2  95 E8  33        -- spm

<00 55> 83 <55>

44 - before the first one
And then repeat the following until the page is full.

D0  1F 00            -- set PC to bootsection for spm to work
D2  B6 01  23 ll     -- in r0,DWDR (ll)
D2  B6 11  23 hh     -- in r1,DWDR (hh)
D2  BF B7  23        -- out SPMCSR,r27 = 01 = SPMEN
D2  95 E8  23        -- spm
D2  96 32  23        -- adiw Z,2


D0 1F 00
D2 01 FC 23 movw r30,r24
D2 BF C7 23 out SPMCSR,r28 = 05 = PGWRT
D2 95 E8 33 spm
<00 55>

D0 1F 00
D2 E1 C1 23 ldi r28,0x11
D2 BF C7 23 out SPMCSR,r28 = 11 = RWWSRE
D2 95 E8 33 spm
<00 55> 83 <55>

*/
