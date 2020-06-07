CC = gcc
CCFLAGS := -c -m32 -march=i386 -nostdlib -ffreestanding -nostartfiles -shared

AS = nasm
ASFLAGS = -felf32

LD = ld
LDFLAGS = -m elf_i386 -shared
PROGRAMS = string.o
SUBDIRS=./kernel ./user
INCLUDES = -I .
 
all:  $(PROGRAMS) 
	for dir in $(SUBDIRS); do \
          $(MAKE) -C $$dir; \
        done
 
clean:
	for dir in $(SUBDIRS); do \
          $(MAKE) clean -C $$dir; \
        done

%.o : %.asm
	$(AS) $(ASFLAGS) $(INCLUDES) $< -o $@
%.o : %.c
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@
%.com : %.asm
	$(AS) $(INCLUDES) $< -o $@
%.s : %.c
	$(CC) $(CCFLAGS) $(INCLUDES) -S $< -o $@