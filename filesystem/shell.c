#include "shell.h"

#include <stdio.h>

#include "disk.h"
#include "file.h"
#include "fsfunc.h"
#include "string.h"

// void simulation(const char *ImgName) {
int main() {
    if (disk_init("./fdd144.img") == false) {
        printf("Error: no such disk.\n");
        return 0;
    }

    if (boot_sector_init()) {
        printf("Successfully read boot sector. Now show the information of boot sector: \n");
        show_boot_sector_info();
    }

    file_init();
    build_all_tree();

    struct File *nowDir = &rootfile;

    char nowPath[50] = {0}, inst[50] = {0};
    strcpy(nowPath, "/");
    // // string nowPath = "/", inst = "";
    // // printf(nowPath << ":";
    printf("%s:", nowPath);

    while (scanf("%s", inst)) {
        if (strcmp(inst, "help") == 0)
            help();
        else if (strcmp(inst, "ls") == 0)
            ls(nowDir);
        else if (strcmp(inst, "tree") == 0)
            tree(nowDir);
        else if (strcmp(inst, "cd") == 0)
            cd(&nowDir, nowPath);
        else if (strcmp(inst, "touch") == 0)
            touch(nowDir);
        else if (strcmp(inst, "mkdir") == 0)
            mkdir(nowDir);
        else if (strcmp(inst, "cat") == 0)
            cat(nowDir);
        else if (strcmp(inst, "write") == 0)
            write(nowDir);
        else if (strcmp(inst, "append") == 0)
            append(nowDir);
        else if (strcmp(inst, "rm") == 0)
            rm(nowDir);
        else if (strcmp(inst, "rmdir") == 0)
            rmdir(nowDir);
        else
            printf("Invalid command.\n");

        // printf(endl;
        // printf(nowPath << ":";
        printf("\n%s:", nowPath);
    }
    disk_close("./fdd144.img");
    return 0;
}