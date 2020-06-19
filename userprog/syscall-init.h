#pragma once

#include "stdint.h"

void syscall_init();

uint32_t sys_get_pid();

uint32_t sys_write(char* s, uint32_t attr);

void sys_read(char* s);

char sys_read_char();

void sys_write_in_pos(char* s, uint32_t attr, uint32_t pos);

void sys_call_intr2a();