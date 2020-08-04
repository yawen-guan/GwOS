CC = gcc
CCFLAGS := -c -m32 -march=i386 -nostdlib -ffreestanding -nostartfiles # -shared -static-libgcc -lgcc
AS = nasm
ASFLAGS = -felf32
LD = ld
LDFLAGS = -m elf_i386  
LIB_KERNEL_DIR = ./lib/kernel
LIB_USER_DIR = ./lib/user
LIB_DIR = ./lib
KERNEL_DIR = ./kernel
DEVICE_DIR = ./device
THREAD_DIR = ./thread
USERPROG_DIR = ./userprog
BUILD_DIR = ./build
BOOT_DIR = ./boot
ACTUAL_USER_DIR = ./userprog/actualuserprogs
INCLUDES = -I $(LIB_KERNEL_DIR) -I $(LIB_USER_DIR) -I $(LIB_DIR) \
		   -I $(DEVICE_DIR) -I $(THREAD_DIR) -I $(USERPROG_DIR) \
		   -I $(KERNEL_DIR)/include -I $(ACTUAL_USER_DIR)

$(BUILD_DIR)/main.o:$(KERNEL_DIR)/main.c 
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/init.o:$(KERNEL_DIR)/init.c
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/interrupt.o: $(KERNEL_DIR)/interrupt.c
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/kernel.o: $(KERNEL_DIR)/kernel.asm
	$(AS) $(ASFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/memory.o: $(KERNEL_DIR)/memory.c
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/debug.o: $(KERNEL_DIR)/debug.c
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/console.o: $(DEVICE_DIR)/console.c
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/ioqueue.o: $(DEVICE_DIR)/ioqueue.c
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/keyboard.o: $(DEVICE_DIR)/keyboard.c
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/timer.o: $(DEVICE_DIR)/timer.c
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/bitmap.o: $(LIB_KERNEL_DIR)/bitmap.c
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/list.o: $(LIB_KERNEL_DIR)/list.c
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/print_asm.o: $(LIB_KERNEL_DIR)/print_asm.asm
	$(AS) $(ASFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/print_c.o: $(LIB_KERNEL_DIR)/print_c.c
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/string.o: $(LIB_DIR)/string.c
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/stdio.o: $(LIB_DIR)/stdio.c
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/syscall.o: $(LIB_USER_DIR)/syscall.c
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/switch.o: $(THREAD_DIR)/switch.asm
	$(AS) $(ASFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/sync.o: $(THREAD_DIR)/sync.c
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/thread.o: $(THREAD_DIR)/thread.c
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/process.o: $(USERPROG_DIR)/process.c
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/tss.o: $(USERPROG_DIR)/tss.c
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/syscall-init.o: $(USERPROG_DIR)/syscall-init.c
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/loader.com: $(BOOT_DIR)/loader.asm
	$(AS) -I $(BOOT_DIR)/include $< -o $@

$(BUILD_DIR)/mbr.com: $(BOOT_DIR)/mbr.asm
	$(AS) -I $(BOOT_DIR)/include $< -o $@

$(BUILD_DIR)/fork.o: $(USERPROG_DIR)/fork.c 
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/wait.o: $(USERPROG_DIR)/wait.c 
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/exit.o: $(USERPROG_DIR)/exit.c 
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/wait_exit.o: $(USERPROG_DIR)/wait_exit.c 
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/prog1.o: $(ACTUAL_USER_DIR)/prog1.c 
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/prog2.o: $(ACTUAL_USER_DIR)/prog2.c 
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/prog3.o: $(ACTUAL_USER_DIR)/prog3.c 
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/prog4.o: $(ACTUAL_USER_DIR)/prog4.c 
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/prog5.o: $(ACTUAL_USER_DIR)/prog5.c 
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/common.o: $(ACTUAL_USER_DIR)/common.c 
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/multiproc.o: $(ACTUAL_USER_DIR)/multiproc.c 
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@


OBJFILE = $(BUILD_DIR)/main.o $(BUILD_DIR)/init.o \
		  $(BUILD_DIR)/interrupt.o $(BUILD_DIR)/timer.o \
		  $(BUILD_DIR)/kernel.o $(BUILD_DIR)/print_c.o \
		  $(BUILD_DIR)/print_asm.o \
          $(BUILD_DIR)/debug.o $(BUILD_DIR)/memory.o \
		  $(BUILD_DIR)/bitmap.o $(BUILD_DIR)/string.o \
		  $(BUILD_DIR)/thread.o $(BUILD_DIR)/list.o \
      	  $(BUILD_DIR)/switch.o $(BUILD_DIR)/console.o \
		  $(BUILD_DIR)/sync.o $(BUILD_DIR)/keyboard.o \
		  $(BUILD_DIR)/ioqueue.o $(BUILD_DIR)/tss.o \
      	  $(BUILD_DIR)/process.o $(BUILD_DIR)/syscall.o \
		  $(BUILD_DIR)/syscall-init.o $(BUILD_DIR)/stdio.o \
		  $(BUILD_DIR)/prog1.o  $(BUILD_DIR)/prog2.o \
		  $(BUILD_DIR)/prog3.o	$(BUILD_DIR)/prog4.o \
		  $(BUILD_DIR)/prog5.o  $(BUILD_DIR)/common.o \
		  $(BUILD_DIR)/fork.o   $(BUILD_DIR)/exit.o \
		  $(BUILD_DIR)/wait.o   $(BUILD_DIR)/multiproc.o 

# 要遵守 调用在前，实现在后，否则虚拟地址会出错
BOOTFILE = $(BUILD_DIR)/mbr.com $(BUILD_DIR)/loader.com

IMG = GwOS.img

all: $(BOOTFILE) $(OBJFILE)
	$(LD) $(LDFLAGS) $(OBJFILE) -Ttext 0xc0001500 -e main -o $(BUILD_DIR)/kernel.com
	dd if=$(BUILD_DIR)/mbr.com of=$(IMG) seek=0 conv=notrunc
	dd if=$(BUILD_DIR)/loader.com of=$(IMG) seek=1 conv=notrunc
	dd if=$(BUILD_DIR)/kernel.com of=$(IMG) seek=8 conv=notrunc

# %.o : %.asm
# 	$(AS) $(ASFLAGS) $(INCLUDES) $< -o $(BUILD_DIR)/$@
# %.o : %.c
# 	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@
# %.com : %.asm
# 	$(AS) $(INCLUDES) $< -o $@
# %.s : %.c
# 	$(CC) $(CCFLAGS) $(INCLUDES) -S $< -o $@

.PHONY : clean
clean :
	cd $(BUILD_DIR) && rm -f ./*
# SUBDIRS=./device ./thread ./lib ./boot ./userprog ./kernel 
 
# clean:
	# for dir in $(SUBDIRS); do \
        #   $(MAKE) clean -C $$dir; \
        # done
