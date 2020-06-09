SUBDIRS=./device ./thread ./lib ./boot ./userprog ./kernel 
IMG = GwOS.img
BOOTDIR = ./boot
KERDIR = ./kernel
LIBDIR = ./lib

 
all: 
	for dir in $(SUBDIRS); do \
          $(MAKE) -C $$dir; \
        done
	dd if=$(BOOTDIR)/mbr.com of=$(IMG) seek=0 conv=notrunc
	dd if=$(BOOTDIR)/loader.com of=$(IMG) seek=1 conv=notrunc
	dd if=$(KERDIR)/kernel.com of=$(IMG) seek=8 conv=notrunc

 
clean:
	for dir in $(SUBDIRS); do \
          $(MAKE) clean -C $$dir; \
        done
