CC = gcc
CCFLAGS := -c -m32 -march=i386 -nostdlib -ffreestanding -nostartfiles # -shared -static-libgcc -lgcc

AS = nasm
ASFLAGS = -felf32

LD = ld
LDFLAGS = -m elf_i386  

INCLUDES = -I ../lib -I ../lib/kernel -I ./include -I ../device -I ../thread -I ../userprog
KERNEL_LIB_DIR = ../lib/kernel
LIB_DIR = ../lib
DEVICE_DIR = ../device
THREAD_DIR = ../thread
USERPROG_DIR = ../userprog
PROGRAM = main.o init.o interrupt.o kernel.o memory.o debug.o
# OBJFILE = # main.o init.o $(THREAD_DIR)/thread.o $(THREAD_DIR)/switch.o $(DEVICE_DIR)/timer.o $(DEVICE_DIR)/keyboard.o interrupt.o memory.o $(KERNEL_LIB_DIR)/bitmap.o $(LIB_DIR)/string.o $(KERNEL_LIB_DIR)/print_c.o $(KERNEL_LIB_DIR)/print_asm.o kernel.o
OBJFILE = main.o init.o debug.o interrupt.o \
      $(DEVICE_DIR)/timer.o kernel.o $(KERNEL_LIB_DIR)/print_c.o \
	  $(KERNEL_LIB_DIR)/print_asm.o \
      memory.o $(KERNEL_LIB_DIR)/bitmap.o \
      $(LIB_DIR)/string.o $(THREAD_DIR)/thread.o $(KERNEL_LIB_DIR)/list.o \
      $(THREAD_DIR)/switch.o $(DEVICE_DIR)/console.o $(THREAD_DIR)/sync.o \
	  $(DEVICE_DIR)/keyboard.o $(DEVICE_DIR)/ioqueue.o $(USERPROG_DIR)/tss.o


# 要遵守 调用在前，实现在后，否则虚拟地址会出错

all: $(PROGRAM)
	$(LD) $(LDFLAGS) $(OBJFILE) -Ttext 0xc0001500 -e main -o kernel.com

%.o : %.asm
	$(AS) $(ASFLAGS) $(INCLUDES) $< -o $@
%.o : %.c
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@
%.com : %.asm
	$(AS) $(INCLUDES) $< -o $@
%.s : %.c
	$(CC) $(CCFLAGS) $(INCLUDES) -S $< -o $@

.PHONY : clean
clean :
	-rm -f *.o *.bin *.com *.elf *.s 