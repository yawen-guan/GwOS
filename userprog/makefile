CC = gcc
CCFLAGS := -c -m32 -march=i386 -nostdlib -ffreestanding -nostartfiles # -shared -static-libgcc -lgcc

AS = nasm
ASFLAGS = -felf32

LD = ld
LDFLAGS = -m elf_i386 # -shared

INCLUDES = -I ../kernel/include -I ../lib -I ../lib/kernel -I ../thread -I ../device -I ../include

PROGRAMS = tss.o  process.o

SUBDIRS = ./actualuserprogs

all: $(PROGRAMS)
	for dir in $(SUBDIRS); do \
          $(MAKE) -C $$dir; \
        done
	# $(LD) $(LDFLAGS) print_asm.o print_c.o -o print.o

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
	for dir in $(SUBDIRS); do \
		$(MAKE) clean -C $$dir; \
	done
