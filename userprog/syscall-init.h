#pragma once

#include "stdint.h"
#include "global.h"

void syscall_init();

uint32_t sys_get_pid();

uint32_t sys_write(char* s, uint32_t attr);

void sys_read(char* s);

char sys_read_char();

void sys_write_in_pos(char* s, uint32_t attr, uint32_t pos);

void sys_call_intr2a();

void sys_read_disk(uint8_t *buf, int secID, bool is_sdb);

void sys_write_disk(const uint8_t* buf, int secID, bool is_sdb);