CC = gcc
CCFLAGS := -c -m32 -march=i386 -nostdlib -ffreestanding -nostartfiles # -shared -static-libgcc -lgcc

AS = nasm
ASFLAGS = -felf32

LD = ld
LDFLAGS = -m elf_i386 # -shared

INCLUDES = -I ../../kernel/include -I ../../lib -I ../../lib/kernel -I ../../thread -I ../../device -I ../../include

PROGRAMS = common.o prog1.o prog2.o prog3.o prog4.o

all: $(PROGRAMS)
	# $(LD) $(LDFLAGS) prog1.o common.o -o print.o

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