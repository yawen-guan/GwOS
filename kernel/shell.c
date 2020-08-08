#include "shell.h"

// #include <stdio.h>

#include "disk.h"
#include "file.h"
#include "fsfunc.h"
#include "stdio.h"
#include "string.h"
#include "syscall.h"
#include "exec.h"

void shell() {

    if (boot_sector_init()) {
        printf("Successfully read boot sector. Now show the information of boot sector: \n");
        show_boot_sector_info();
    }

    // file_init();
    // build_all_tree();

    struct File *nowDir = &rootfile;

    char nowPath[50] = {0}, inst[50] = {0};
    strcpy(nowPath, "/");
    printf("%s:", nowPath);

    while (true) {
        scanf("%s", inst);
        if (strcmp(inst, "help") == 0)
            shell_help();
        else if (strcmp(inst, "ls") == 0)
            shell_ls(nowDir);
        else if (strcmp(inst, "tree") == 0)
            shell_tree(nowDir);
        else if (strcmp(inst, "cd") == 0)
            shell_cd(&nowDir, nowPath);
        else if (strcmp(inst, "touch") == 0)
            shell_touch(nowDir);
        else if (strcmp(inst, "mkdir") == 0)
            shell_mkdir(nowDir);
        else if (strcmp(inst, "cat") == 0)
            shell_cat(nowDir);
        else if (strcmp(inst, "write") == 0)
            shell_write(nowDir, false, 0);
        else if (strcmp(inst, "append") == 0)
            shell_append(nowDir);
        else if (strcmp(inst, "rm") == 0)
            shell_rm(nowDir);
        else if (strcmp(inst, "rmdir") == 0)
            shell_rmdir(nowDir);
        else if (strcmp(inst, "exec") == 0)
            shell_exec(nowDir);
        else if(strcmp(inst, "sdb") == 0)
            shell_write_program_to_sdb();
        else
            printf("Invalid command.\n");
        printf("\n%s:", nowPath);
    }
    disk_close();
    exit(0);
}