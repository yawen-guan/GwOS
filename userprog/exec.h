#pragma once

#include "stdint.h"
#include "filesystem.h"

int32_t sys_execv(struct ActiveFile* acfile, const char* argv[]);