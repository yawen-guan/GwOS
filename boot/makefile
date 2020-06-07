AS = nasm
ASFLAGS = -felf32

INCLUDES = -I ./include
BUILD_DIR = ../build

all: mbr.com loader.com 

%.com : %.asm
	$(AS) $(INCLUDES) $< -o $@

.PHONY : clean
clean :
	-rm -f *.o *.bin *.com *.elf *.s 