// NonVolatile.c


enum {SPMEN=1, PGERS=3, PGWRT=5, RWWSRE=0x11, BLBSET = 0x09, SIGRD = 0x21};


u8 SPMCSR;

void ReadConfigBits(u8 pmc, u8 index, u8 *dest) {
  u16 outr0 = 0xB800 | ((DWDRreg() & 0x30) << 5) | (DWDRreg() & 0xF);
  DwSend(Bytes(
    0x66,                                  // Register or memory access mode
    0xD0, 0x10, 29,                        // Set up to set registers starting from r29
    0xD1, 0x10, 32,                        // Register limit is 32 (i.e. stop at r31)
    0xC2, 0x05, 0x20,                      // Select reigster write mode and start
    pmc, index, 0,                         // Set r29 to BLBSET LPM flags, and Z to desired fuse index
    0xD0, hi(BootSect()), lo(BootSect()),  // Set PC inside the boot section to enable spm
    0x64,                                  // Non volatile memory access mode
    0xD2, 0xBF, 0xD7, 0x23,                // OUT SPMCSR,r29
    0xD2, 0x95, 0xC8, 0x23,                // LPM: Reads fuse or lock bits to R0
    0xD2, hi(outr0), lo(outr0), 0x23       // OUT DRDW,r0
  ));
  DwReceive(dest, 1);
}

void DumpConfig() {
  u8 cb;
  u8 hfuse;
  u8 efuse;

  // Show Known device info
  Ws(Name()); Wsl(".");
  Ws("IO regs:     ");       Wx(32,2); Ws(".."); Wx(IoregSize()+31,2); Ws("     ("); Wd(IoregSize(),1); Wsl(" bytes).");
  Ws("SRAM:        ");       Wx(IoregSize()+32,3); Ws(".."); Wx(31 + IoregSize() + SramSize(), 3); Ws("   ("); Wd(SramSize(),1); Wsl(" bytes).");
  Ws("Flash:       0000.."); Wx(FlashSize()-1, 4); Ws(" ("); Wd(FlashSize(),1); Wsl(" bytes).");
  Ws("EEPROM:      000..");  Wx(EepromSize()-1, 3); Ws("   ("); Wd(EepromSize(),1); Wsl(" bytes).");

  // Dump uninterpreted fuse and lock bit values
  ReadConfigBits(BLBSET, 0, &cb);    Ws("Fuses:       low "); Wx(cb,2);
  ReadConfigBits(BLBSET, 3, &hfuse); Ws(", high "); Wx(hfuse,2);
  ReadConfigBits(BLBSET, 2, &efuse); Ws(", extended "); Wx(efuse,2); Wsl(".");
  ReadConfigBits(BLBSET, 1, &cb);    Ws("Lock bits:   "); Wx(cb,2); Wsl(".");
  if (SigRead()) {
    ReadConfigBits(SigRead(), 0, &cb); Ws("Signature:   "); Wx(cb,2);
    ReadConfigBits(SigRead(), 2, &cb); Ws(" "); Wx(cb,2);
    ReadConfigBits(SigRead(), 4, &cb); Ws(" "); Wx(cb,2); Wsl(".");
    ReadConfigBits(SigRead(), 1, &cb); Ws("Osc cal:     "); Wx(cb,2); Wsl(".");
  }

  // Interpret boot sector information
  if (BootFlags()) {
    int bootsize = 0;
    int bootvec  = 0;
    switch (BootFlags()) {
      case 1:  bootsize = 128 << (3 - ((efuse/2) & 3));  bootvec = efuse & 1; break; // atmega88 & 168
      case 2:  bootsize = 256 << (3 - ((hfuse/2) & 3));  bootvec = hfuse & 1; break; // atmega328
      default: Fail("Invalid BootFlags global data setting.");
    }

    bootsize *= 2;  // Bootsize is in words but we report in bytes.

    Ws("Boot loader: ");
    Wx(FlashSize()-bootsize, 4); Ws(".."); Wx(FlashSize()-1, 4);
    Ws(" ("); Wd(bootsize,1); Wsl(" bytes).");
    Ws("Reset vec:   ");
    if (bootvec) Wsl("0."); else {Wx(FlashSize()-bootsize, 4); Wsl(".");}
  }
}


u16 InInstruction (u16 io, u16 reg) {return 0xB000 | ((io << 5) & 0x600) | ((reg << 4) & 0x01F0) | (io & 0x000F);}
u16 OutInstruction(u16 io, u16 reg) {return 0xB800 | ((io << 5) & 0x600) | ((reg << 4) & 0x01F0) | (io & 0x000F);}


u8 ReadSPMCSR() {
  u8  spmcsr;
  u16 inSPMCSR = InInstruction (0x57, 30);      // in r30,SPMCSR
  u16 outR30   = OutInstruction(DWDRreg(), 30); // out DWDR,r30
  DwSend(Bytes(
    0x64,                                       // Non volatile memory access mode
    0xD2, hi(inSPMCSR), lo(inSPMCSR), 0x23,     // in r30,SPMCSR
    0xD2, hi(outR30),   lo(outR30),   0x23      // out DWDR,r30
  ));
  DwReceive(&spmcsr, 1);
  return spmcsr;
}



// Smallest boot size is 128 words on atmega 88 and 168.
// Smallest boot size on atmega328 is 256 words.

void DwReadFlash(int addr, int len, u8 *buf) {
  int limit = addr + len;
  if (limit > FlashSize()) {Fail("Attempt to read beyond end of flash.");}
  while (addr < limit) {
    int length = min(limit-addr, 64); // Read no more than 64 bytes at a time so PC remains in valid address space.
    //Ws("ReadFlashBlock at $"); Wx(addr,1); Ws(", length $"); Wx(length,1); Wl();

    DwSend(Bytes(
      0x66,                // Access registers/memory mode
      0xD0, 0x10, 0x1E,    // PC := 0x1E (Address r30/r31 aka Z)
      0xD1, 0x10, 0x20,    // BP := 0x20
      0xC2, 0x05, 0x20,    // Start writing Z
      lo(addr), hi(addr),  // Z := addr
      0xD0, hi(BootSect()), lo(BootSect()), // Set PC that allows access to all of flash
      0xD1, 0x10 | hi(BootSect()+2*length), 0x10 | lo(BootSect()+2*length), // Set BP to stop after length reads
      0xC2, 0x02, 0x20     // Set flash read mode and start reading
    ));
    DwReceive(buf, length);
    addr += length;
    buf  += length;
  }
}


void EraseFlashPage(u16 a) { // a = byte address of first word of page
  // Uses r29, r30, r31
  Assert((a & (PageSize()-1)) == 0);
  DwSend(Bytes(
    0x66,                                 // Register or memory access mode
    0xD0, 0x10, 29,                       // Set up to set registers starting from r29
    0xD1, 0x10, 32,                       // Register limit is 32 (i.e. stop at r31)
    0xC2, 0x05, 0x20,                     // Select reigster write mode and start
    PGERS, lo(a), hi(a),                  // r29=op, Z=first byte address of page
    0xD0, hi(BootSect()), lo(BootSect()), // Set PC that allows access to all of flash
    0x64,                                 // Non volatile memory access mode
    0xD2, 0xBF, 0xD7, 0x23,               // out SPMCSR,r29 (3=PGERS, 5=PGWRT)
    0xD2, 0x95, 0xE8, 0x33                // spm
  ));
  DwSync();
}

void RenableRWW() {
  if (BootSect()) {
    u16 inr29 = InInstruction(DWDRreg(), 29);
    DwSend(Bytes(
      0xD0, hi(BootSect()), lo(BootSect()),     // Set PC that allows access to all of flash
      0xD2, hi(inr29), lo(inr29), 0x23, RWWSRE, // in r29,DWDR. r29:= RWWSRE
      0xD2, 0xBF, 0xD7, 0x23,                   // out SPMCSR,r29
      0xD2, 0x95, 0xE8, 0x33                    // spm
    ));
    DwFlush();
  }
}

void InitPageBuffer(u16 a) {
  DwSend(Bytes(
    0x66,                 // Register or memory access mode
    0xD0, 0x10, 29,       // Set up to set registers starting from r29
    0xD1, 0x10, 32,       // Register limit is 32 (i.e. stop at r31)
    0xC2, 0x05, 0x20,     // Select reigster write mode and start
    SPMEN, lo(a), hi(a),  // r29 = op (write next page buffer word), Z = first byte address of page
    0x64                  // Non volatile memory access mode
  ));
}


void LoadPageBuffer(const u8 *buf) {
  const u8 *p     = buf;
  const u8 *limit = buf + PageSize();
  u16       inr0  = InInstruction(DWDRreg(), 0); // 0xB000 | ((DWDRreg() & 0x30) << 5) | (DWDRreg() & 0xF);
  u16       inr1  = InInstruction(DWDRreg(), 1); // 0xB010 | ((DWDRreg() & 0x30) << 5) | (DWDRreg() & 0xF);
  while (p < limit) {
    DwSend(Bytes(
      0xD2, hi(inr0), lo(inr0), 0x23, *(p++), // in r0,DWDR (low byte)
      0xD2, hi(inr1), lo(inr1), 0x23, *(p++), // in r1,DWDR (high byte)
      0xD0, hi(BootSect()), lo(BootSect()),   // Set PC that allows access to all of flash
      0xD2, 0xBF, 0xD7, 0x23,                 // out SPMCSR,r29 (write next page buffer word)
      0xD2, 0x95, 0xE8, 0x23,                 // spm
      0xD2, 0x96, 0x32, 0x23                  // adiw Z,2
    ));
  }
}


void ProgramPage(u16 a) {
  DwSend(Bytes(
    0x66,                                 // Register or memory access mode
    0xD0, 0x10, 29,                       // Set up to set registers starting from r29
    0xD1, 0x10, 32,                       // Register limit is 32 (i.e. stop at r31)
    0xC2, 0x05, 0x20,                     // Select reigster write mode and start
    PGWRT, lo(a), hi(a),                  // r29 = op (page write), Z = first byte address of page
    0xD0, hi(BootSect()), lo(BootSect()), // Set PC that allows access to all of flash
    0x64,                                 // Non volatile memory access mode
    0xD2, 0xBF, 0xD7, 0x23,               // out SPMCSR,r29 (3=PGERS, 5=PGWRT)
    0xD2, 0x95, 0xE8, 0x23                // spm
  ));
  // Wait for page programming to complete
  while ((ReadSPMCSR() & 0x1F) != 0) Wc('.');
}




void ShowPageStatus(u16 a, char *msg) {
  Ws("$"); Wx(a,4); Ws(" - $"); Wx(a+PageSize()-1,4);
  Wc(' '); Ws(msg); Ws(".                "); Wr();
}




void WriteFlashPage(u16 a, const u8 *buf) {
  // Uses r0, r1, r29, r30, r31

  u8 page[MaxFlashPageSize];
  Assert(PageSize() <= sizeof(page));

  RenableRWW();

  DwReadFlash(a, PageSize(), page);

  if (memcmp(buf, page, PageSize()) == 0) {
    ShowPageStatus(a, "unchanged");
    return;
  }

  //  // Debugging, what's different???
  //  Wl();
  //  Ws("Difference found for page at $"); Wx(a,4); Wsl(".");
  //  Wsl("Existing page contains:");
  //  DumpBytes(0, PageSize(), page); Wl();
  //  Wsl("New page contains:");
  //  DumpBytes(0, PageSize(), buf); Wl();
  //  Fail("Aborting");


  int erase = 0;
  for (int i=0; i<PageSize(); i++) {
    if (~page[i] & buf[i]) {erase=1; break;}
  }

  if (erase) {
    ShowPageStatus(a, "erasing");
    EraseFlashPage(a);
  }

  memset(page, 0xff, PageSize());
  if (memcmp(buf, page, PageSize()) == 0) {
    return;
  }

  ShowPageStatus(a, "loading page buffer");
  InitPageBuffer(a);
  LoadPageBuffer(buf);

  ShowPageStatus(a, "programming");
  ProgramPage(a);

  RenableRWW();
}




u8 pageBuffer[MaxFlashPageSize] = {0};

void WriteFlash(u16 addr, const u8 *buf, int length) {

  Assert(addr + length <= FlashSize());
  Assert(length >= 0);
  if (length == 0) return;

  u8 r0and1[2];
  u8 r29;

  DwReadRegisters(r0and1, 0, 2);
  DwReadRegisters(&r29,  29, 1);

  int pageOffsetMask = PageSize()-1;
  int pageBaseMask   = ~ pageOffsetMask;

  if (addr & pageOffsetMask) {

    // buf starts in the middle of a page

    int partBase   = addr & pageBaseMask;
    int partOffset = addr & pageOffsetMask;
    int partLength = min(PageSize()-partOffset, length);

    DwReadFlash(partBase, PageSize(), pageBuffer);
    memcpy(pageBuffer+partOffset, buf, partLength);
    WriteFlashPage(partBase, pageBuffer);

    addr   += partLength;
    buf    += partLength;
    length -= partLength;
  }

  Assert(length == 0  ||  ((addr & pageOffsetMask) == 0));

  // Write whole pages

  while (length >= PageSize()) {
    WriteFlashPage(addr, buf);
    addr   += PageSize();
    buf    += PageSize();
    length -= PageSize();
  }

  // Write any remaining partial page

  if (length) {
    Assert(length > 0);
    Assert(length < PageSize());
    Assert((addr & pageOffsetMask) == 0);
    DwReadFlash(addr, PageSize(), pageBuffer);
    memcpy(pageBuffer, buf, length);
    WriteFlashPage(addr, pageBuffer);
  }

  Ws("                                       \r");

  DwWriteRegisters(r0and1, 0, 2);
  DwWriteRegisters(&r29,  29, 1);
}



void DumpFlashBytesCommand() {
  int length = 128; // Default byte count to display
  ParseDumpParameters("dump flash bytes", FlashSize(), &FBaddr, &length);

  u8 buf[length];
  DwReadFlash(FBaddr, length, buf);

  DumpBytes(FBaddr, length, buf);
  FBaddr += length;
}




void DumpFlashWordsCommand() {
  int length = 128; // Default byte count to display
  ParseDumpParameters("dump flash words", FlashSize(), &FWaddr, &length);

  u8 buf[length];
  DwReadFlash(FWaddr, length, buf);

  DumpWords(FWaddr, length, buf);
  FWaddr += length;
}




void WriteFlashBytesCommand() {
  u8 buf[16];
  int addr;
  int len;
  ParseWriteParameters("Flash write", &addr, FlashSize(), buf, &len);
  if (len) {WriteFlash(addr, buf, len);}
}






void DwReadEEPROM(int addr, int len, u8 *buf)
{
  u8 quint[5];

  u16  eearh = EEARH() ? EEARH() : EEDR(); // Dump address high harmelssly in data reg on chips with no address high
  u16 outEearlR30 = OutInstruction(EEARL(), 30);
  u16 outEearhR31 = OutInstruction(eearh,   31);
  u16 outEecrR29  = OutInstruction(EECR(),  29);
  u16 inR0Eedr    = InInstruction (EEDR(),   0);
  u16 inR1Eedr    = InInstruction (EEDR(),   1);
  u16 inR2Eedr    = InInstruction (EEDR(),   2);
  u16 inR3Eedr    = InInstruction (EEDR(),   3);
  u16 inR4Eedr    = InInstruction (EEDR(),   4);
  u16 adiwR30x1   = 0x9631;

  int limit = addr + len;
  if (limit > EepromSize()) {Fail("Attempt to read beyond end of EEPROM.");}

  // Preload registers: r29 = EECR read flag, r31:r30 = EEPROM address
  DwSend(Bytes(
    0x66,               // Register or memory access mode
    0xD0, 0x10, 29,     // Set up to set registers starting from r29
    0xD1, 0x10, 32,     // Register limit is 32 (i.e. stop at r31)
    0xC2, 0x05, 0x20,   // Select reigster write mode and start
    0x01,               // r29 := 1
    lo(addr), hi(addr)  // r31:r30 := address
  ));

  // Read all requested bytes
  while (addr < limit) {

    DwSend(Bytes(
      0x64,                                           // Non volatile memory access mode

      0xD2, hi(outEearlR30), lo(outEearlR30), 0x23,   // out  EEARL,r30 - Set read address
      0xD2, hi(outEearhR31), lo(outEearhR31), 0x23,   // out  EEARH,r31 (writes EEDR harmlessly on chips without EEARH)
      0xD2, hi(outEecrR29),  lo(outEecrR29),  0x23,   // out  EECR,r29  - start EEPROM read
      0xD2, hi(adiwR30x1),   lo(adiwR30x1),   0x23,   // adi  r31:r30,1 - Address next eeprom byte
      0xD2, hi(inR0Eedr),    lo(inR0Eedr),    0x23,   // in   r0,EEDR   - load EEPROM value to R0

      0xD2, hi(outEearlR30), lo(outEearlR30), 0x23,   // out  EEARL,r30 - Set read address
      0xD2, hi(outEearhR31), lo(outEearhR31), 0x23,   // out  EEARH,r31 (writes EEDR harmlessly on chips without EEARH)
      0xD2, hi(outEecrR29),  lo(outEecrR29),  0x23,   // out  EECR,r29  - start EEPROM read
      0xD2, hi(adiwR30x1),   lo(adiwR30x1),   0x23,   // adi  r31:r30,1 - Address next eeprom byte
      0xD2, hi(inR1Eedr),    lo(inR1Eedr),    0x23,   // in   r1,EEDR   - load EEPROM value to R1

      0xD2, hi(outEearlR30), lo(outEearlR30), 0x23,   // out  EEARL,r30 - Set read address
      0xD2, hi(outEearhR31), lo(outEearhR31), 0x23,   // out  EEARH,r31 (writes EEDR harmlessly on chips without EEARH)
      0xD2, hi(outEecrR29),  lo(outEecrR29),  0x23,   // out  EECR,r29  - start EEPROM read
      0xD2, hi(adiwR30x1),   lo(adiwR30x1),   0x23,   // adi  r31:r30,1 - Address next eeprom byte
      0xD2, hi(inR2Eedr),    lo(inR2Eedr),    0x23,   // in   r2,EEDR   - load EEPROM value to R2

      0xD2, hi(outEearlR30), lo(outEearlR30), 0x23,   // out  EEARL,r30 - Set read address
      0xD2, hi(outEearhR31), lo(outEearhR31), 0x23,   // out  EEARH,r31 (writes EEDR harmlessly on chips without EEARH)
      0xD2, hi(outEecrR29),  lo(outEecrR29),  0x23,   // out  EECR,r29  - start EEPROM read
      0xD2, hi(adiwR30x1),   lo(adiwR30x1),   0x23,   // adi  r31:r30,1 - Address next eeprom byte
      0xD2, hi(inR3Eedr),    lo(inR3Eedr),    0x23,   // in   r3,EEDR   - load EEPROM value to R3

      0xD2, hi(outEearlR30), lo(outEearlR30), 0x23,   // out  EEARL,r30 - Set read address
      0xD2, hi(outEearhR31), lo(outEearhR31), 0x23,   // out  EEARH,r31 (writes EEDR harmlessly on chips without EEARH)
      0xD2, hi(outEecrR29),  lo(outEecrR29),  0x23,   // out  EECR,r29  - start EEPROM read
      0xD2, hi(adiwR30x1),   lo(adiwR30x1),   0x23,   // adi  r31:r30,1 - Address next eeprom byte
      0xD2, hi(inR4Eedr),    lo(inR4Eedr),    0x23,   // in   r4,EEDR   - load EEPROM value to R4

      0x66,                                           // Register or memory access mode
      0xD0, 0x10, 0x00,                               // Set up to get registers starting from r0
      0xD1, 0x10, 0x05,                               // Register limit is 5 (i.e. stop at r4)
      0xC2, 0x01, 0x20                                // Select reigster read mode and start
    ));
    DwReceive(quint, 5);

    int i = 0;
    while (i < 5  &&  addr < limit) {
      *buf = quint[i];
      buf++;
      addr++;
      i++;
    }
  }
}


void DwWriteEEPROM(int addr, int len, u8 *buf)
{
  //Ws("DwWriteEEPROM($"); Wx(addr,4); Ws(", "); Wd(len,1); Wsl(", buf);");

  int eearh = EEARH() ? EEARH() : EEDR(); // Dump address high harmelssly in data reg on chips with no address high
  u16 inR0Dwdr    = InInstruction (DWDRreg(),  0);
  u16 outEedrR0   = OutInstruction(EEDR(),     0);
  u16 outEecrR28  = OutInstruction(EECR(),    28);
  u16 outEecrR29  = OutInstruction(EECR(),    29);
  u16 outEearlR30 = OutInstruction(EEARL(),   30);
  u16 outEearhR31 = OutInstruction(eearh,     31);
  u16 adiwR30x1   = 0x9631;

  int limit = addr + len;
  if (limit > EepromSize()) {Fail("Attempt to write beyond end of EEPROM.");}

  // Preload registers:
  //   r28     - EEMWE flag (4)
  //   r29     - EEWE flag (2)
  //   r31:r30 - Initial EEPROM address

  DwSend(Bytes(
    0x66,                     // Register or memory access mode
    0xD0, 0x10, 28,           // Set up to set registers starting from r28
    0xD1, 0x10, 32,           // Register limit is 32 (i.e. stop at r31)
    0xC2, 0x05, 0x20,         // Select reigster write mode and start
    4, 2, lo(addr), hi(addr)  // r28 := 4, r29 := 2, r31:r30 := address
  ));

  // Write all requested bytes
  while (addr < limit) {
    DwSend(Bytes(
      0x64,                                               // Non volatile memory access mode
      0xD2, hi(outEearlR30), lo(outEearlR30), 0x23,       // out  EEARL,r30 - Set read address
      0xD2, hi(outEearhR31), lo(outEearhR31), 0x23,       // out  EEARH,r31 (does nothing on chips without EEARH)
      0xD2, hi(inR0Dwdr),    lo(inR0Dwdr),    0x23, *buf, // in   r0,DWDR (read next byte being written to EEPROM)
      0xD2, hi(outEedrR0),   lo(outEedrR0),   0x23,       // out  EEDR,R0
      0xD2, hi(outEecrR28),  lo(outEecrR28),  0x23,       // out  EECR,r28  - Master EEPROM program enable
      0xD2, hi(outEecrR29),  lo(outEecrR29),  0x23,       // out  EECR,r29  - Enable EEPROM program write
      0xD2, hi(adiwR30x1),   lo(adiwR30x1),   0x23        // adi  r31:r30,1 - Address next eeprom byte
    ));
    DwFlush();
    buf++;
    addr++;
    delay(5); // Allow eeprom write to complete.
  }
}




void DumpEEPROMBytesCommand() {
  int length = 32; // Default byte count to display
  ParseDumpParameters("dump EEPROM bytes", EepromSize(), &EBaddr, &length);

  u8 buf[length];
  DwReadEEPROM(EBaddr, length, buf);

  DumpBytes(EBaddr, length, buf);
  EBaddr += length;
}




void DumpEEPROMWordsCommand() {
  int length = 32; // Default byte count to display
  ParseDumpParameters("dump EEPROM words", EepromSize(), &EWaddr, &length);

  u8 buf[length];
  DwReadEEPROM(EWaddr, length, buf);

  DumpWords(EWaddr, length, buf);
  EWaddr += length;
}




void WriteEEPROMBytesCommand() {
  u8 buf[16];
  int addr;
  int len;
  ParseWriteParameters("EEPROM write", &addr, EepromSize(), buf, &len);
  if (len) {DwWriteEEPROM(addr, len, buf);}
}
