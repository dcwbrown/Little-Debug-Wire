/// UserInterface.c

//        User interface
//
//        Reads lines, parses them into commands and parameters, and calls the
//        appropriate handler procedures.





void HelpCommand();

void PCommand()            {PC = ReadInstructionAddress("PC");}
void BPCommand()           {BP = ReadInstructionAddress("BP");}
void BCCommand()           {BP = -1;}
void QuitCommand()         {QuitRequested = 1;}
void DisableCommand()      {DwDisable(); Exit(0);}
void QuitPauseCommand()    {Exit(0);}
void FailCommand()         {Fail("FailCommand ...");}
void EmptyCommand()        {Sb(); if (!DwEoln()) {HelpCommand();}}
void VerboseCommand()      {Verbose = 1;}
void TimerEnableCommand()  {TimerEnable = 1;}
void TimerDisableCommand() {TimerEnable = 0;}


void DisassemblyPrompt() {
  u8 buf[4];  // Enough for a 2 word instruction
  DwReadFlash(PC, 4, buf);
  Uaddr = PC + DisassembleInstruction(PC, buf);
  Wt(HasLineNumbers ? 60 : 40);
}


void TraceCommand() {
  int count = 1;
  Sb(); if (IsDwDebugNumeric(NextCh())) {count = ReadNumber(0);}
  if (count < 1) Fail("Trace count should be at least 1");
  DwTrace();
  while (count > 1) {DisassemblyPrompt(); Wl(); DwTrace(); count--;}
}


struct {char *name; char *help; int requiresConnection; void (*handler)();} Commands[] = {
  {"b",           "Set breakpoint",                                1, BPCommand},
  {"bc",          "Clear breakpoint",                              1, BCCommand},
  {"d",           "Dump data bytes",                               1, DumpDataBytesCommand},
  {"dw",          "Dump data words",                               1, DumpDataWordsCommand},
  {"f",           "Dump flash bytes",                              1, DumpFlashBytesCommand},
  {"fw",          "Dump flash words",                              1, DumpFlashWordsCommand},
  {"l",           "Load file",                                     1, LoadFileCommand},
  {"g",           "Go",                                            1, GoCommand},
  {"p",           "PC set / query",                                1, PCommand},
  {"q",           "Quit ",                                         0, QuitCommand},
  {"r",           "Display registers",                             1, RegistersCommand},
  {"s",           "Stack",                                         1, StackCommand},
  {"t",           "Trace",                                         1, TraceCommand},
  {"te",          "Timer enable",                                  1, TimerEnableCommand},
  {"td",          "Timer disable",                                 1, TimerDisableCommand},
  {"u",           "Unassemble",                                    1, UnassembleCommand},
  {"h",           "Help",                                          0, HelpCommand},
  {"reset",       "Reset processor",                               1, DwReset},
  {"config",      "Display fuses, lockbits, signature and osccal", 1, DumpConfig},
  {"qdd",         "Quit and temporarily disable debugWIRE",        1, DisableCommand},
  {"qpause",      "Quit leaving processor paused",                 1, QuitPauseCommand},
  {"help",        "Help",                                          0, HelpCommand},
  {"verbose",     "Set verbose mode",                              0, VerboseCommand},
  {"gdbserver",   "Start server for GDB",                          1, GdbserverCommand},
  {"",            0,                                               0, EmptyCommand},
};


int IsInteractive = 0;


void HelpCommand() {
  for (int i=0; Commands[i].help; i++) {
    Ws("  "); Ws(Commands[i].name); Wt(12); Ws("- "); Wsl(Commands[i].help);
  }
}


void HandleCommand(const char *cmd) {
  for (int i=0; i<countof(Commands); i++) {
    if (!strcmp(cmd, Commands[i].name)) {
      Commands[i].handler();
      return;
    }
  }
  Ws("Unrecognised command: '"); Ws(cmd); Wsl("'.");
}


void ParseAndHandleCommand() {
  char command[20];

  Sb(); if (IsCommandSeparator(NextCh())) {SkipCh(); Sb();}

  if (Eof()) {if (IsInteractive) {Wl();}  QuitCommand();}
  else       {Ra(ArrayAddressAndLength(command)); HandleCommand(command);}

  SkipWhile(NotDwEoln); SkipWhile(IsDwEoln);
}


void Prompt() {
  if (BufferTotalContent() == 0  &&  IsInteractive) {
    if (OutputPosition == 0) {DisassemblyPrompt();}
    Wt(HasLineNumbers ? 60 : 40); Ws("> "); Flush();
    HorizontalPosition = 0;  // Let SimpleOutput know user returned to column 1
  } else {
    if (OutputPosition) {Wl();}
  }
}


void UI() {
  DwConnect();
  PreloadInput(GetCommandParameters());
  while (1) {
    if (QuitRequested) {DwGo(); /*usb_close(Port); Port = 0; */ return;}
    if (BufferTotalContent() == 0) {IsInteractive = Interactive(Input);}
    if (IsInteractive) {
      if (setjmp(FailPoint)) {SkipWhile(NotEoln); SkipEoln();}
    }
    Prompt();
    ParseAndHandleCommand();
  }
}