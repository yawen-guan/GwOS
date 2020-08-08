LD = ld
LDFLAGS = -m elf_i386
CC = gcc
CCFLAGS = -c -m32 -march=i386 -nostdlib -ffreestanding -nostartfiles

LIB_KERNEL_DIR = ../lib/kernel
LIB_USER_DIR = ../lib/user
LIB_DIR = ../lib
KERNEL_DIR = ../kernel
DEVICE_DIR = ../device
THREAD_DIR = ../thread
USERPROG_DIR = ../userprog
BUILD_DIR = ../build
BOOT_DIR = ../boot
FS_DIR = ../filesystem
ACTUAL_USER_DIR = ../userprog/actualuserprogs
INCLUDES = -I $(LIB_KERNEL_DIR) -I $(LIB_USER_DIR) -I $(LIB_DIR) \
		   -I $(DEVICE_DIR) -I $(THREAD_DIR) -I $(USERPROG_DIR) \
		   -I $(KERNEL_DIR)/include -I $(ACTUAL_USER_DIR) \
		   -I $(FS_DIR)

OBJFILE = $(BUILD_DIR)/print_c.o \
		  $(BUILD_DIR)/print_asm.o \
		  $(BUILD_DIR)/string.o $(BUILD_DIR)/syscall.o $(BUILD_DIR)/stdio.o \


all:
	${CC} ${CCFLAGS} ${INCLUDES} -o helloworld.o helloworld.c
	${LD} ${LDFLAGS} -e main helloworld.o ${OBJFILE} -o helloworld


# %.o : %.asm
# 	$(AS) $(ASFLAGS) $(INCLUDES) $< -o $(BUILD_DIR)/$@
# %.o : %.c
# 	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@
# %.com : %.asm
# 	$(AS) $(INCLUDES) $< -o $@
# %.s : %.c
# 	$(CC) $(CCFLAGS) $(INCLUDES) -S $< -o $@

# SUBDIRS=./device ./thread ./lib ./boot ./userprog ./kernel 
 
# clean:
	# for dir in $(SUBDIRS); do \
        #   $(MAKE) clean -C $$dir; \
        # done



# SEC_CNT=$(ls -l helloworld | awk '{printf(%d, ($5+511)/512)}')

# if [[ -f helloworld ]];then
#    dd if=./helloworld of=../GwOS.img bs=512 \
#    count=${SEC_CNT} seek=300 conv=notrunc
# fi
