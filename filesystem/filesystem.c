#include "filesystem.h"

void filesystem_init() {
    file_init();
    build_all_tree();
}