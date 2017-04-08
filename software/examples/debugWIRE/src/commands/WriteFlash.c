// WriteFlash.c


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
  ReadConfigBits(BLBSET, 0, &cb); Ws("Fuse low byte:      "); Wx(cb,2); Wl();
  ReadConfigBits(BLBSET, 2, &cb); Ws("Fuse extended byte: "); Wx(cb,2); Wl();
  ReadConfigBits(BLBSET, 3, &cb); Ws("Fuse high byte:     "); Wx(cb,2); Wl();
  ReadConfigBits(BLBSET, 1, &cb); Ws("Lock Bits:          "); Wx(cb,2); Wl();
  ReadConfigBits(SIGRD,  0, &cb); Ws("Signature byte 1:   "); Wx(cb,2); Wl();
  ReadConfigBits(SIGRD,  2, &cb); Ws("Signature byte 2:   "); Wx(cb,2); Wl();
  ReadConfigBits(SIGRD,  4, &cb); Ws("Signature byte 3:   "); Wx(cb,2); Wl();
  ReadConfigBits(SIGRD,  1, &cb); Ws("RC Oscillator Cal:  "); Wx(cb,2); Wl();
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


void EraseFlashPage(u16 a) { // a = byte address of first word of page
  // Uses r29, r30, r31

  Ws("Erasing flash page "); Wx(a,4); Wl();

  Assert((a & (PageSize()-1)) == 0);
  DwTransferSync(ByteArrayLiteral(
    0x66, 0xD0, 0, 29, 0xD1, 0, 32,      // Set PC=29, BP=32 - address registers r29 through r31
    0xC2, 5, 0x20, PGERS, lo(a), hi(a),  // r29=op, Z=first byte address of page
    0xD0, 0x3F, 0,                       // Set PC=$3F00, inside the boot section to enable spm
    0x64, 0xD2, 0xBF, 0xD7, 0x23,        // out SPMCSR,r29 (3=PGERS, 5=PGWRT)
    0xD2, 0x95, 0xE8, 0x33               // spm
  ));
}

void RenableRWW() {
  Ws("RenableRWW called. "); LogSPMCSR();
  DwTransfer(ByteArrayLiteral(
    0x66, 0xD0, 0, 29, 0xD1, 0, 30,      // Set PC=29, BP=30 - address registers r29
    0xC2, 5, 0x20, RWWSRE,               // r29 = op (reenable RWW section)
    0xD0, 0x3F, 0x00,                    // Set PC to bootsection for spm to work
    0x64, 0xD2, 0xBF, 0xD7, 0x23,        // out SPMCSR,r29 (3=PGERS, 5=PGWRT)
    0xD2, 0x95, 0xE8, 0x33),             // spm
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

void AddWordToPageBuffer(u16 w) {
  u16 inr0 = 0xB000 | ((DWDRreg() & 0x30) << 5) | (DWDRreg() & 0xF);
  u16 inr1 = 0xB010 | ((DWDRreg() & 0x30) << 5) | (DWDRreg() & 0xF);
  //Ws("AddWordToPageBuffer called. "); LogSPMCSR();
  DwTransfer(ByteArrayLiteral(
    0x64,                                   // D2 using instruction
    0xD2, hi(inr0), lo(inr0), 0x23, lo(w),  // in r0,DWDR (low byte)
    0xD2, hi(inr1), lo(inr1), 0x23, hi(w),  // in r1,DWDR (high byte)
    0xD0, 0x3F, 0x00,                       // Set PC to bootsection for spm to work
    0xD2, 0xBF, 0xD7, 0x23,                 // out SPMCSR,r29 (write next page buffer word)
    0xD2, 0x95, 0xE8, 0x23,                 // spm
    0xD2, 0x96, 0x32, 0x23), 0,0);          // adiw Z,2
}

void ProgramPage(u16 a) {
  Ws("ProgramPage called. "); LogSPMCSR();
  DwTransfer(ByteArrayLiteral(
    0x66, 0xD0, 0, 29, 0xD1, 0, 32,      // Set PC=29, BP=32 - address registers r29 through r31
    0xC2, 5, 0x20, PGWRT, lo(a), hi(a),  // r29 = op (page write), Z = first byte address of page
    0xD0, 0x3F, 0x00,                    // Set PC to bootsection for spm to work
    0x64, 0xD2, 0xBF, 0xD7, 0x23,        // out SPMCSR,r29 (3=PGERS, 5=PGWRT)
    0xD2, 0x95, 0xE8, 0x23),             // spm
    0,0
  );
  Ws("SPM executed. "); LogSPMCSR();
  while ((SPMCSR & 0x1F) != 0) {Ws("Waiting for SPM to complete. "); LogSPMCSR();}
}

void ProgramFlashPage(u16 a, const u8 *buf) {
  const u16 *p = (const u16*)buf;
  Ws("ProgramFlashPage called. "); LogSPMCSR();
  InitPageBuffer(a);
  for (int i=0; i<PageSize()/2; i++) {AddWordToPageBuffer(*(p++));}
  ProgramPage(a);
  RenableRWW();
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
