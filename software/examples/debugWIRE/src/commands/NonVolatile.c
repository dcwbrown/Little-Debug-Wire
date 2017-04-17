// NonVolatile.c


enum {SPMEN=1, PGERS=3, PGWRT=5, RWWSRE=0x11, BLBSET = 0x09, SIGRD = 0x21};


u8 SPMCSR;

void ReadConfigBits(u8 pmc, u8 index, u8 *dest) {
  u16 outr0 = 0xB800 | ((DWDRreg() & 0x30) << 5) | (DWDRreg() & 0xF);
  DwTransfer(ByteArrayLiteral(
    0x66, 0xD0, 0, 29, 0xD1, 0, 32,          // Set PC=29, BP=32 - address registers r29 through r31
    0xC2, 5, 0x20, pmc, index, 0,            // Set r29 to BLBSET LPM flags, and Z to desired fuse index
    0xD0, 0x3F, 0,                           // Set PC=$3F00, inside the boot section to enable spm
    0x64, 0xD2, 0xBF, 0xD7, 0x23,            // OUT SPMCSR,r29
    0x64, 0xD2, 0x95, 0xC8, 0x23,            // LPM: Reads fuse or lock bits to R0
    0x64, 0xD2, hi(outr0), lo(outr0), 0x23), // OUT DRDW,r0
    dest, 1
  );
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
    int bootsize;
    int bootvec;
    switch (BootFlags()) {
      case 1:  bootsize = 128 << (3 - ((efuse/2) & 3));  bootvec = efuse & 1; break; // atmega88 & 168
      case 2:  bootsize = 256 << (3 - ((hfuse/2) & 3));  bootvec = hfuse & 1; break; // atmega328
      default: Fail("Invalid BootFlags global data setting.");
    }
    Ws("Boot loader: ");
    Wx(FlashSize()-bootsize, 4); Ws(".."); Wx(FlashSize()-1, 4);
    Ws(" ("); Wd(bootsize,1); Wsl(" bytes).");
    Ws("Reset vec:   ");
    if (bootvec) Wsl("0."); else {Wx(FlashSize()-bootsize, 4); Wsl(".");}
  }
}

void ReadSPMCSR() {
  DwTransfer(ByteArrayLiteral(
    0x66, 0xD0, 0,0x1e, 0xD1, 0,0x20,       // Set PC=0x001E, BP=0x0020 (i.e. address register Z)
    0xC2, 5, 0x20, 0x57, 0,                 // Address SPMCSR
    0xD0, 0,0, 0xD1, 0, 2,                  // Set PC=0, BP=2*length
    0xC2, 0, 0x20),                         // Start the read
    &SPMCSR, 1
  );
}

void LogSPMCSR() {Ws("SPMCSR = "); ReadSPMCSR(); Wx(SPMCSR,2); Wl();}


// Smallest boots size is 128 words on atmega 88 and 168.
// Smallest boot size on atmega328 is 256 words.

void DwReadFlash(int addr, int len, u8 *buf) {
  int limit = addr + len;
  if (limit > FlashSize()) {Fail("Attempt to read beyond end of flash.");}
  while (addr < limit) {
    int length = min(limit-addr, 64); // Read no more than 64 bytes at a time so PC remains in valid address space.
    //Ws("ReadFlashBlock at $"); Wx(addr,1); Ws(", length $"); Wx(length,1); Wl();
    DwTransfer(ByteArrayLiteral(
      0x66, 0xD0, 0,0x1e, 0xD1, 0,0x20,           // Set PC=0x001E, BP=0x0020 (i.e. address register Z)
      0xC2, 5, 0x20, lo(addr),hi(addr),           // Write addr to Z
      0xD0, hi(BootSect()), lo(BootSect()),       // Set PC that allows access to all of flash
      0xD1, hi(BootSect()+2*length), lo(BootSect()+2*length), // Set BP to stop after length reads
      0xC2, 2, 0x20),                             // Read length bytes from flash starting at first
      buf, length
    );
    addr += length;
    buf  += length;
  }
}


void EraseFlashPage(u16 a) { // a = byte address of first word of page
  // Uses r29, r30, r31

  Ws("Erasing flash page "); Wx(a,4); Wl();

  Assert((a & (PageSize()-1)) == 0);
  DwTransferSync(ByteArrayLiteral(
    0x66, 0xD0, 0, 29, 0xD1, 0, 32,       // Set PC=29, BP=32 - address registers r29 through r31
    0xC2, 5, 0x20, PGERS, lo(a), hi(a),   // r29=op, Z=first byte address of page
    0xD0, hi(BootSect()), lo(BootSect()), // Set PC that allows access to all of flash
    0x64, 0xD2, 0xBF, 0xD7, 0x23,         // out SPMCSR,r29 (3=PGERS, 5=PGWRT)
    0xD2, 0x95, 0xE8, 0x33                // spm
  ));
}

void RenableRWW() {
  Ws("RenableRWW called. "); LogSPMCSR();
  DwTransfer(ByteArrayLiteral(
    0x66, 0xD0, 0, 29, 0xD1, 0, 30,       // Set PC=29, BP=30 - address registers r29
    0xC2, 5, 0x20, RWWSRE,                // r29 = op (reenable RWW section)
    0xD0, hi(BootSect()), lo(BootSect()), // Set PC that allows access to all of flash
    0x64, 0xD2, 0xBF, 0xD7, 0x23,         // out SPMCSR,r29 (3=PGERS, 5=PGWRT)
    0xD2, 0x95, 0xE8, 0x33),              // spm
    0,0
  );
}

void InitPageBuffer(u16 a) {
  Ws("InitPageBuffer called. "); LogSPMCSR();
  DwTransfer(ByteArrayLiteral(
    0x66, 0xD0, 0, 29, 0xD1, 0, 32,      // Set PC=29, BP=32 - address registers r29 through r31
    0xC2, 5, 0x20, SPMEN, lo(a), hi(a),  // r29 = op (write next page buffer word), Z = first byte address of page
    0x64),
    0,0
  );
}


void LoadPageBuffer(const u8 *buf) {
  const u8 *p     = buf;
  const u8 *limit = buf + PageSize();
  u16       inr0  = 0xB000 | ((DWDRreg() & 0x30) << 5) | (DWDRreg() & 0xF);
  u16       inr1  = 0xB010 | ((DWDRreg() & 0x30) << 5) | (DWDRreg() & 0xF);
  while (p < limit) {
    DwTransfer(ByteArrayLiteral(
      0x64,                                   // D2 using instruction
      0xD2, hi(inr0), lo(inr0), 0x23, *(p++), // in r0,DWDR (low byte)
      0xD2, hi(inr1), lo(inr1), 0x23, *(p++), // in r1,DWDR (high byte)
      0xD0, hi(BootSect()), lo(BootSect()),   // Set PC that allows access to all of flash
      0xD2, 0xBF, 0xD7, 0x23,                 // out SPMCSR,r29 (write next page buffer word)
      0xD2, 0x95, 0xE8, 0x23,                 // spm
      0xD2, 0x96, 0x32, 0x23,                 // adiw Z,2
      0xD2, hi(inr0), lo(inr0), 0x23, *(p++), // in r0,DWDR (low byte)
      0xD2, hi(inr1), lo(inr1), 0x23, *(p++), // in r1,DWDR (high byte)
      0xD2, 0xBF, 0xD7, 0x23,                 // out SPMCSR,r29 (write next page buffer word)
      0xD2, 0x95, 0xE8, 0x23,                 // spm
      0xD2, 0x96, 0x32, 0x23,                 // adiw Z,2
      0xD2, hi(inr0), lo(inr0), 0x23, *(p++), // in r0,DWDR (low byte)
      0xD2, hi(inr1), lo(inr1), 0x23, *(p++), // in r1,DWDR (high byte)
      0xD2, 0xBF, 0xD7, 0x23,                 // out SPMCSR,r29 (write next page buffer word)
      0xD2, 0x95, 0xE8, 0x23,                 // spm
      0xD2, 0x96, 0x32, 0x23,                 // adiw Z,2
      0xD2, hi(inr0), lo(inr0), 0x23, *(p++), // in r0,DWDR (low byte)
      0xD2, hi(inr1), lo(inr1), 0x23, *(p++), // in r1,DWDR (high byte)
      0xD2, 0xBF, 0xD7, 0x23,                 // out SPMCSR,r29 (write next page buffer word)
      0xD2, 0x95, 0xE8, 0x23,                 // spm
      0xD2, 0x96, 0x32, 0x23),                // adiw Z,2
    0,0);
  }
}



void ProgramPage(u16 a) {
  Ws("ProgramPage called. "); LogSPMCSR();
  DwTransfer(ByteArrayLiteral(
    0x66, 0xD0, 0, 29, 0xD1, 0, 32,      // Set PC=29, BP=32 - address registers r29 through r31
    0xC2, 5, 0x20, PGWRT, lo(a), hi(a),  // r29 = op (page write), Z = first byte address of page
    0xD0, hi(BootSect()), lo(BootSect()),// Set PC that allows access to all of flash
    0x64, 0xD2, 0xBF, 0xD7, 0x23,        // out SPMCSR,r29 (3=PGERS, 5=PGWRT)
    0xD2, 0x95, 0xE8, 0x23),             // spm
    0,0
  );
  Ws("SPM executed. "); LogSPMCSR();
  while ((SPMCSR & 0x1F) != 0) {Ws("Waiting for SPM to complete. "); LogSPMCSR();}
}

void ProgramFlashPage(u16 a, const u8 *buf) {
  //const u16 *p = (const u16*)buf;
  Ws("ProgramFlashPage called. "); LogSPMCSR();
  InitPageBuffer(a);
  //for (int i=0; i<PageSize()/2; i++) {AddWordToPageBuffer(*(p++));}
  LoadPageBuffer(buf);
  ProgramPage(a);
  RenableRWW();
}


int InInstruction (int io,  int reg) {return 0xB000 | ((io & 0x30) << 5) | (reg << 4) | (io & 0xF);}
int OutInstruction(int io,  int reg) {return 0xB800 | ((io & 0x30) << 5) | (reg << 4) | (io & 0xF);}

void DwReadEEPROM(int addr, int len, u8 *buf)
{
  int eearh = EEARH() ? EEARH() : EEDR(); // Dump address high harmelssly in data reg on chips with no address high
  u16 outEearlR30 = OutInstruction(EEARL(),   30);
  u16 outEearhR31 = OutInstruction(eearh,     31);
  u16 outEecrR29  = OutInstruction(EECR(),    29);
  u16 inR0Eedr    = InInstruction (EEDR(),     0);
  u16 inR1Eedr    = InInstruction (EEDR(),     1);
  u16 inR2Eedr    = InInstruction (EEDR(),     2);
  u16 inR3Eedr    = InInstruction (EEDR(),     3);
  u16 inR4Eedr    = InInstruction (EEDR(),     4);
  u16 adiwR30x1   = 0x9631;

  int limit = addr + len;
  if (limit > EepromSize()) {Fail("Attempt to read beyond end of EEPROM.");}

  // Preload registers: r29 = EECR read flag, r31:r30 = EEPROM address
  DwTransfer(ByteArrayLiteral(
    0x66,
    0xD0, 0,0x1D, 0xD1, 0,0x20,            // Set PC=0x001d, BP=0x0020: address registers 29-31
    0xC2, 5, 0x20, 1, lo(addr),hi(addr),   // r29 := 1, r31:r30 := address
    0x64),
    0,0
  );

  // Read all requested bytes
  while (addr < limit) {
    // Read five bytes at a time for performance (largest qty that fits 128 byte transfer limit)
    u8 quint[5];
    DwTransfer(ByteArrayLiteral(
      0x64,
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

      0x66, 0xD0, 0, 0, 0xD1, 0, 5, 0xC2, 1, 0x20),   // Read registers r0,r1,r2,r3,r4
      quint, 5
    );

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
  int eearh = EEARH() ? EEARH() : EEDR(); // Dump address high harmelssly in data reg on chips with no address high
  u16 inR0Dwdr    = InInstruction (DWDRreg(), 0);
  u16 outEedrR0   = OutInstruction(EEDR(),    0);
  u16 outEecrR28  = OutInstruction(EECR(),   28);
  u16 outEecrR29  = OutInstruction(EECR(),   29);
  u16 outEearlR30 = OutInstruction(EEARL(),  30);
  u16 outEearhR31 = OutInstruction(eearh,    31);
  u16 adiwR30x1   = 0x9631;

  int limit = addr + len;
  if (limit > EepromSize()) {Fail("Attempt to write beyond end of EEPROM.");}

  // Preload registers:
  //   r28     - EEMWE flag (4)
  //   r29     - EEWE flag (2)
  //   r31:r30 - Initial EEPROM address
  DwTransfer(ByteArrayLiteral(
    0x66,
    0xD0, 0,0x1C, 0xD1, 0,0x20,              // Set PC=0x001c, BP=0x0020: address registers 28-31
    0xC2, 5, 0x20, 4, 2, lo(addr),hi(addr),  // r28 := 4, r29 := 2, r31:r30 := address
    0x64),
    0,0
  );

  // Write all requested bytes
  while (addr < limit) {
    DwTransfer(ByteArrayLiteral(
      0xD2, hi(outEearlR30), lo(outEearlR30), 0x23,       // out  EEARL,r30 - Set read address
      0xD2, hi(outEearhR31), lo(outEearhR31), 0x23,       // out  EEARH,r31 (does nothing on chips without EEARH)
      0xD2, hi(inR0Dwdr),    lo(inR0Dwdr),    0x23, *buf, // in   r0,DWDR (read next byte being written to EEPROM)
      0xD2, hi(outEedrR0),   lo(outEedrR0),   0x23,       // out  EEDR,R0
      0xD2, hi(outEecrR28),  lo(outEecrR28),  0x23,       // out  EECR,r28  - Master EEPROM program enable
      0xD2, hi(outEecrR29),  lo(outEecrR29),  0x23,       // out  EECR,r29  - Enable EEPROM program write
      0xD2, hi(adiwR30x1),   lo(adiwR30x1),   0x23),      // adi  r31:r30,1 - Address next eeprom byte
      0,0
    );
    buf++;
    addr++;
  }
}





//void ProgramFlashPage(u16 a, const u8 *buf) {
//  const u8 *p = buf;
//  u16       inr0 = 0xB000 | ((DWDRreg() & 0x30) << 5) | (DWDRreg() & 0xF);
//  u16       inr1 = 0xB010 | ((DWDRreg() & 0x30) << 5) | (DWDRreg() & 0xF);
//  u8        SPMCSR;
//
//  DwReadAddr(0x57, 1, &SPMCSR);
//  Wl(); Ws("SPMCSR: "); Wx(SPMCSR, 2); Wsl(".");
//
//  Wsl("Transfer page to page buffer.");
//  DwTransfer(ByteArrayLiteral(
//    0x66, 0xD0, 0, 29, 0xD1, 0, 32,      // Set PC=29, BP=32 - address registers r29 through r31
//    0xC2, 5, 0x20, SPMEN, lo(a), hi(a),  // r29 = op (write next page buffer word), Z = first byte address of page
//    0x64), 0,0);
//
//  for (int i=0; i<PageSize()/2; i++) {
//    DwTransfer(ByteArrayLiteral(
//      0xD2, hi(inr0), lo(inr0), 0x23, *(p++), // in r0,DWDR (low byte)
//      0xD2, hi(inr1), lo(inr1), 0x23, *(p++), // in r1,DWDR (high byte)
//      0xD0, 0x3F, 0x00,                       // Set PC to bootsection for spm to work
//      0xD2, 0xBF, 0xD7, 0x23,                 // out SPMCSR,r29 (write next page buffer word)
//      0xD2, 0x95, 0xE8, 0x23,                 // spm
//      0xD2, 0x96, 0x32, 0x23), 0,0);          // adiw Z,2
//  }
//
//  if (0) {
//
//    // Not programming a read while write page.
//    // The processor will halt during the programming and we'll get a
//    // 0x55 back at completion.
//    DwTransferSync(ByteArrayLiteral(
//      0x66, 0xD0, 0, 29, 0xD1, 0, 32,      // Set PC=29, BP=32 - address registers r29 through r31
//      0xC2, 5, 0x20, PGWRT, lo(a), hi(a),  // r29 = op (page write), Z = first byte address of page
//      0xD0, 0x3F, 0x00,                    // Set PC to bootsection for spm to work
//      0x64, 0xD2, 0xBF, 0xD7, 0x23,        // out SPMCSR,r29 (3=PGERS, 5=PGWRT)
//      0xD2, 0x95, 0xE8, 0x33               // spm
//    ));
//
//  } else {
//
//    DwReadAddr(0x57, 1, &SPMCSR);
//    Ws("SPMCSR: "); Wx(SPMCSR, 2); Wsl(".");
//
//    // Enable RWW section.
//    Wsl("Enabling RWW section.");
//    DwTransfer(ByteArrayLiteral(
//      0x66, 0xD0, 0, 29, 0xD1, 0, 30,      // Set PC=29, BP=30 - address registers r29
//      0xC2, 5, 0x20, RWWSRE,               // r29 = op (reenable RWW section)
//      0xD0, 0x3F, 0x00,                    // Set PC to bootsection for spm to work
//      0x64, 0xD2, 0xBF, 0xD7, 0x23,        // out SPMCSR,r29 (3=PGERS, 5=PGWRT)
//      0xD2, 0x95, 0xE8, 0x33),             // spm
//      0,0
//    );
//
//    DwReadAddr(0x57, 1, &SPMCSR);
//    Ws("SPMCSR: "); Wx(SPMCSR, 2); Wsl(".");
//
//    // Programming a read while write page.
//    // The processor will keep running during programming.
//    // We need to monitor SPMEN in SPMCR and wait for it to
//    // go low indicating end of programming, then we need to
//    // reenable the RWW section.
//
//    Wsl("Programming page.");
//    DwTransfer(ByteArrayLiteral(
//      0x66, 0xD0, 0, 29, 0xD1, 0, 32,      // Set PC=29, BP=32 - address registers r29 through r31
//      0xC2, 5, 0x20, PGWRT, lo(a), hi(a),  // r29 = op (page write), Z = first byte address of page
//      0xD0, 0x3F, 0x00,                    // Set PC to bootsection for spm to work
//      0x64, 0xD2, 0xBF, 0xD7, 0x23,        // out SPMCSR,r29 (3=PGERS, 5=PGWRT)
//      0xD2, 0x95, 0xE8, 0x23),             // spm
//      0,0
//    );
//    // Show PC - has it advanced from 1F00? Yes. To 1F04.
//    Ws("PC: "); Wx(DwTransferReadWord(ByteArrayLiteral(0xF0)), 2); Wsl(".");
//    // Now wait for SPMEN to go low
//    do {
//      DwReadAddr(0x57, 1, &SPMCSR);
//      if (SPMCSR & 1) {Ws("SPMCSR: "); Wx(SPMCSR, 2); Wsl(".");}
//    } while (SPMCSR & 1);
//    // Renable RWW section.
//    DwTransfer(ByteArrayLiteral(
//      0x66, 0xD0, 0, 29, 0xD1, 0, 30,      // Set PC=29, BP=30 - address registers r29
//      0xC2, 5, 0x20, RWWSRE,               // r29 = op (reenable RWW section)
//      0xD0, 0x3F, 0x00,                    // Set PC to bootsection for spm to work
//      0x64, 0xD2, 0xBF, 0xD7, 0x23,        // out SPMCSR,r29 (3=PGERS, 5=PGWRT)
//      0xD2, 0x95, 0xE8, 0x33),             // spm
//      0,0
//    );
//
//  }
//}




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

 ShowPageStatus(a, "programming");
 ProgramFlashPage(a, buf);
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
