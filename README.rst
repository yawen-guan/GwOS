====
GwOS
====

The Operating System Experiment in SYSU

.. contents:: :local:

Description
-----------
An Operating system with FAT12 filesystem. It can run simple user program now!

Features
--------
* FAT12 filesystem
* Execute user program
* simple hard disk drive: reading elf files from hard disk 
* simple keyboard drive: supporting only the main keyboard, with a ring bidirectional buffer
* memory paging: with two-level page tables
* Linux-like Privilege Level: 0 for kernel, 3 for normal users.
* Interruption: using 8259A abd 8253
* Processes: 6 status process model
* Synchronization: lock and semaphore
* System call: get_pid, read, write, call, malloc, free 
* User library: scanf, printf
