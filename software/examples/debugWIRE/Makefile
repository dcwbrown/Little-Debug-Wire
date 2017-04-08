#PATH := /bin

# set variable TOOLCHAIN_DIR to cross compile
# example: https://github.com/raspberrypi/tools
# make TOOLCHAIN_DIR=<tools>/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64

# Uncomment the following line to remove the Linux dependency on GTK
#NOFILEDIALOG:=1

TARGET := dwdebug

ifdef WINDIR
BINARY := $(TARGET).exe
else

BINARY := $(TARGET)

#autodetect FILEDIALOG options for Linux based on existance of gtk+-3.0
CC = gcc
ifdef  TOOLCHAIN_DIR
       CC := $(wildcard $(TOOLCHAIN_DIR)/bin/*-gcc)
       NOFILEDIALOG:=1
endif
ifndef NOFILEDIALOG
       FILEDIALOG := $(shell pkg-config --exists gtk+-3.0 && echo \`pkg-config --cflags --libs gtk+-3.0\`)
endif
ifeq ($(strip $(FILEDIALOG)),)
       FILEDIALOG = -DNOFILEDIALOG
endif

endif


ifeq ($(shell uname), Linux)
	USBFLAGS = `libusb-config --cflags`
	USBLIBS = `libusb-config --libs`
	EXE_SUFFIX =
	OSFLAG = -D LINUX
else ifeq ($(shell uname), Darwin)
	USBFLAGS = `libusb-config --cflags`
	USBLIBS = `libusb-config --libs`
	EXE_SUFFIX =
	OSFLAG = -D LINUX
else
	USBFLAGS = -I D:\projects\gcc\libusb-win32-bin-1.2.6.0\include
	USBLIBS = -L D:\projects\gcc\libusb-win32-bin-1.2.6.0\lib\gcc -lusb
	EXE_SUFFIX = .exe
	OSFLAG = -D WIN
endif


FLAGS = $(USBFLAGS) $(USBLIBS) -I ../../library


.PHONY: all
.PHONY: clean
.PHONY: run
.PHONY: install

ifndef TOOLCHAIN_DIR
#all: clean run
all: run

run: $(BINARY)
	./$(BINARY) f0,q
endif

$(BINARY): */*/*.c */*.c Makefile
ifdef WINDIR
ifdef NOFILEDIALOG
	i686-w64-mingw32-gcc -std=gnu99 -Wall -o $(BINARY) -Dwindows -DNOFILEDIALOG src/$(TARGET).c $(FLAGS) -lKernel32 -lWinmm -lWs2_32
else
	i686-w64-mingw32-gcc -std=gnu99 -Wall -o $(BINARY) -Dwindows src/$(TARGET).c $(FLAGS) -lKernel32 -lWinmm -lComdlg32 -lWs2_32
	#i686-w64-mingw32-gcc -g -oo -std=gnu99 -Wall -o $(BINARY) -Dwindows src/$(TARGET).c -lKernel32 -lComdlg32
endif
else
	$(CC) -std=gnu99 -g -fno-pie -rdynamic -fPIC -Wall -o $(BINARY) src/$(TARGET).c $(FLAGS) $(FILEDIALOG)
endif
	ls -lap $(BINARY)

clean:
	-rm -rf *.o *.map *.list $(BINARY)

ifndef TOOLCHAIN_DIR
install:
	cp -p $(BINARY) /usr/local/bin
endif
