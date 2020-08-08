#include "fsfunc.h"

// #include <stdio.h>

#include "disk.h"
#include "exec.h"
#include "file.h"
#include "stdio.h"
#include "string.h"
#include "syscall.h"

char* msg_help =
    "help: print the instructions and their usage\n\r"
    "\n\r"
    "        usage: help \n\r"
    "\n\r"
    "ls: list directory contents \n\r"
    "\n\r"
    "        usage: ls -l                list information of unhidden files in the directory\n\r"
    "               ls -al               list information of all files in the directory\n\r"
    "\n\r"
    "tree: list contents of directories in a tree-like format.\n\r"
    "\n\r"
    "        usage: tree -L level        level is the max display depth of the directory tree.\n\r"
    "                                    For example, 'tree -L 3'\n\r"
    "                                    By default, hidden files are not shown\n\r"
    "               tree -aL level       Also show the hidden files'\n\r"
    "\n\r"
    "cd: change directory\n\r"
    "\n\r"
    "        usage: cd relative_path     For example, 'cd subdir/subsubdir' \n\r"
    "               cd absolute_path     Absolute path is begin with '/'\n\r"
    "                                    For example, 'cd /dir/subdir' \n\r"
    "\n\r"
    "touch: create a new empty file\n\r"
    "\n\r"
    "        usage: touch filename\n\r"
    "\n\r"
    "mkdir: create a new empty directory\n\r"
    "\n\r"
    "        usage: mkdir dirname\n\r"
    "\n\r"
    "cat: print the content of the file\n\r"
    "\n\r"
    "        usage: cat filename\n\r"
    "\n\r"
    "write: write the content of the file\n\r"
    "\n\r"
    "        usage : write filename\n\r"
    "\n\r"
    "append: copy the contentt of a file and paste it in the tail of another file\n\r"
    "\n\r"
    "        usage : append file1 file2 The content of file1 is appended to file2\n\r"
    "\n\r"
    "rm: delete files\n\r"
    "\n\r"
    "        usage : rm filename Delete a file\n\r "
    "                rm -r dirname Delete the entire directory recursively\n\r"
    "\n\r"
    "rmdir: delete a empty directory\n\r"
    "\n\r"
    "        usage : rmdir dirname\n\r"
    "\n\r";

void build_all_tree() {
    rootfile.FstSon = build_tree(&rootfile, 0);
}

struct File* build_tree(struct File* fa, int32_t clusID) {  // clusID = 0: root directory
    struct File *head = NULL, *now = NULL;
    uint8_t buff[512];
    uint16_t secID = (clusID == 0) ? 19 : clusID + ClusToSec;

    while (true) {
        if (clusID == 0 && secID == 33) break;
        if (clusID != 0 && clusID >= 0xFF8 && clusID <= 0xFFF) break;
        read_sector(secID, buff);

        for (int i = 0; i < 512; i += 32) {                         //directory
            if (buff[i + 0] == 0xE5 || buff[i + 0] == 0) continue;  // deleted or unused;
            if (now == NULL) {
                now = make_file_from_sector(fa, buff, i);
                head = now;
            } else {
                now->NextSib = make_file_from_sector(fa, buff, i);
                now = now->NextSib;
            }
            if (now->Attr == 0x10 && now->Filename[0] != 0x2E) {  // not . or ..
                now->FstSon = build_tree(now, now->FstClus);
            }
        }

        if (clusID == 0)
            secID++;
        else {
            clusID = get_fat_next_cluster(clusID);
            secID = clusID + ClusToSec;
        }
    }
    return head;
}

void list_directory(struct File* dir, bool ShowHid) {
    struct File* now = dir->FstSon;
    while (now != NULL) {
        file_print_all(now, ShowHid);
        now = now->NextSib;
    }
}

void tree_directory(struct File* dir, bool ShowHid, int maxLayer) {
    printf(".\n");
    struct File* now = dir->FstSon;
    while (now != NULL) {
        // if(now -> Filename[0] != 0x2E)
        tree_directory_with_spacenum(0, now, ShowHid, maxLayer);
        now = now->NextSib;
    }
}

void tree_directory_with_spacenum(int SpaceNum, struct File* p, bool ShowHid, int maxLayer) {
    if (p->used == false || p->Filename[0] == 0x2E) return;
    if (ShowHid == false && p->Attr == 0x02) return;
    for (int i = 0; i < SpaceNum; i++)
        printf(" ");
    printf("|---");
    file_print_name(p);
    printf("\n");

    if (SpaceNum == (maxLayer - 1) * 4) return;
    struct File* son = p->FstSon;
    while (son != NULL) {
        tree_directory_with_spacenum(SpaceNum + 4, son, ShowHid, maxLayer);
        son = son->NextSib;
    }
}

struct File* is_file_exist(struct File* dir, const char* filename) {
    struct File* now = dir->FstSon;
    while (now != NULL) {
        bool same = true;
        for (int i = 0; i < 11; i++)
            if (now->Filename[i] != filename[i]) {
                same = false;
                break;
            }
        if (same == true) return now;
        now = now->NextSib;
    }
    return NULL;
}

bool create_empty_file(struct File* dir, char* Filename, uint8_t Attr, uint16_t WrtTime, uint16_t WrtDate) {
    struct File* p = is_file_exist(dir, Filename);
    if (p != NULL) {
        if (p->Attr == 0x20)
            printf("There is a directory with the same name in this path.\n");
        else
            printf("There is a file with the same name in this path\n");
        return false;
    }
    uint16_t clusID = find_empty_cluster();
    if (clusID == -1) return false;
    struct File* file = get_empty_file();
    init_file(file, dir, Filename, Attr, WrtTime, WrtDate, clusID, 0);
    if (!create_dir_entry(dir, file)) {
        // delete file;
        file->used = false;
        return false;
    }
    write_fat_next_cluster(clusID, 0xFFF);
    file->NextSib = dir->FstSon;
    dir->FstSon = file;
    return true;
}

bool create_empty_directory(struct File* dir, char* Filename, uint16_t WrtTime, uint16_t WrtDate) {
    struct File* p = is_file_exist(dir, Filename);
    if (p != NULL) {
        if (p->Attr == 0x20)
            printf("There is a directory with the same name in this path.\n");
        else
            printf("There is a file with the same name in this path\n");
        return false;
    }

    uint16_t clusID = find_empty_cluster();
    if (clusID == -1) return false;
    struct File* subdir = get_empty_file();  //new File(dir, Filename, 0x10, WrtTime, WrtDate, clusID, 0);
    init_file(subdir, dir, Filename, 0x10, WrtTime, WrtDate, clusID, 0);
    if (!create_dir_entry(dir, subdir)) {
        // delete subdir;
        subdir->used = false;
        return false;
    }
    write_fat_next_cluster(clusID, 0xFFF);
    if (!write_empty_directory(dir, subdir)) {
        // delete subdir;
        subdir->used = false;
        return false;
    }
    subdir->NextSib = dir->FstSon;
    dir->FstSon = subdir;
    return true;
}

bool write_file(struct ActiveFile* acfile) {
    if (acfile->Type == ReadOnly) return false;
    int size = 0, id = 0, cnt = 0;
    uint8_t buff[512];
    struct AFBLNode* now = get_empty_AFBLNode();
    acfile->head = now;
    int clusID = acfile->file->FstClus;

    char s[1024];
    printf("Please enter content: \n");
    scanf("%s", s);
    int s_len = strlen(s);
    for (int i = 0; i < s_len; i++) {  // strange strlen
        if (id < BytePerSec) buff[id] = s[i];
        if (id == BytePerSec - 1) {
            buff[id + 1] = 0;
            cnt++;
            if (cnt > 1) {
                int newClusID = get_fat_next_cluster(clusID);
                if (newClusID >= 0xFF8 && newClusID <= 0xFFF)
                    newClusID = append_a_cluster(clusID);
                if (newClusID == -1) return false;
                clusID = newClusID;
                now->next = get_empty_AFBLNode();
                if (now->next == NULL) return false;
                now = now->next;
            }
            init_AFBLNode(now, buff, NULL);
        }
        id = (id + 1) % BytePerSec;
        size++;
    }
    if (id > 0) {
        for (int i = id; i < BytePerSec; i++)
            buff[i] = 0x00;
        cnt++;

        if (cnt > 1) {
            int newClusID = get_fat_next_cluster(clusID);
            if (newClusID >= 0xFF8 && newClusID <= 0xFFF)
                newClusID = append_a_cluster(clusID);
            if (newClusID == -1) return false;
            clusID = newClusID;
            now->next = get_empty_AFBLNode();
            if (now->next == NULL) return false;
            now = now->next;
        }
        init_AFBLNode(now, buff, NULL);
    }
    write_fat_next_cluster(clusID, 0xFFF);
    acfile->buffsize = size;
    return true;
}

bool write_file_from_sda(struct ActiveFile* acfile, uint32_t filesize) {
    if (acfile->Type == ReadOnly) return false;
    int size = 0, id = 0, cnt = 0;
    uint8_t buff[512];
    struct AFBLNode* now = get_empty_AFBLNode();
    acfile->head = now;
    int clusID = acfile->file->FstClus;
    uint32_t secCnt = DIV_ROUND_UP(filesize, 512);

    for (int i = 0; i < secCnt; i++) {
        if (i >= 1) {
            int newClusID = get_fat_next_cluster(clusID);
            if (newClusID >= 0xFF8 && newClusID <= 0xFFF)
                newClusID = append_a_cluster(clusID);
            if (newClusID == -1) return false;
            clusID = newClusID;
            now->next = get_empty_AFBLNode();
            if (now->next == NULL) return false;
            now = now->next;
        }
        read_disk(buff, 300 + i, false);
        init_AFBLNode(now, buff, NULL);
    }

    // printf("sdb %d sector = \n", 32);
    // read_disk(buff, 32, true);
    // for (int j = 0; j < 512; j++)
    //     printf("%x ", buff[j]);
    // printf("\n");
    // printf("secID = %d\n", clusID + ClusToSec);

    acfile->buffsize = filesize;
    return true;
}

void read_file(struct ActiveFile* acfile) {
    uint16_t clusID = acfile->file->FstClus;
    int size = 0;
    uint8_t buff[512];

    while (true) {
        if (clusID >= 0xFF8 && clusID <= 0xFFF) break;
        read_sector(clusID + ClusToSec, buff);
        for (int i = 0; i < BytePerSec; i++) {
            printf("%c", buff[i]);
            size++;
            if (size == acfile->file->FileSize) break;
        }
        clusID = get_fat_next_cluster(clusID);
    }
    printf("\n");
}

bool delete_file_from_fa(struct File* fa, struct File* file) {
    struct File *now = fa->FstSon, *pre = NULL;
    bool found = false;
    while (now != NULL) {
        if (now == file) {
            if (pre != NULL)
                pre->NextSib = now->NextSib;
            else
                fa->FstSon = now->NextSib;
            found = true;
        }
        pre = now;
        now = now->NextSib;
    }
    if (found == false) {
        if (file->Attr == 0x10) {
            printf("No such directory in this path.\n");
        } else {
            printf("No such file in this directory.\n");
        }
        return false;
    }
    return true;
}

bool delete_file(struct File* dir, struct File* file) {
    if (!delete_file_from_fa(dir, file)) return false;
    clear_file(file);
    file->used = false;
    // delete file;
    return true;
}

bool is_dir_empty(struct File* dir) {
    struct File* son = dir->FstSon;
    while (son != NULL) {
        if (son->Filename[0] != 0x2E) return false;
        son = son->NextSib;
    }
    return false;
}

bool delete_directory(struct File* fa, struct File* dir, bool rc) {
    if (rc == false && is_dir_empty(dir) == true) {
        printf("Error: This directory is not empty.\n");
        return false;
    }
    if (!delete_file_from_fa(fa, dir)) return false;
    if (rc == true) {
        struct File* son = dir->FstSon;
        while (son != NULL) {
            if (son->Filename[0] != 0x2E) {
                if (son->Attr == 0x10)
                    delete_directory(dir, son, true);
                else
                    delete_file(dir, son);
            }
            son = son->NextSib;
        }
    }
    clear_file(dir);
    dir->used = false;
    // delete dir;
    return true;
}

struct File* change_directory(struct File* dir, const char* subdirname) {
    if (strcmp(subdirname, ".          ") == 0) return dir;
    if (strcmp(subdirname, "..         ") == 0) {
        return dir->Fa;
    }
    struct File* now = dir->FstSon;
    while (now != NULL) {
        bool same = true;
        for (int i = 0; i < 11; i++)
            if (now->Filename[i] != subdirname[i]) {
                same = false;
                break;
            }
        if (same == true) {
            return now;
        }
        now = now->NextSib;
    }
    return NULL;
}

bool append_file(struct File* file1, struct File* file2) {
    // auto buff1 = MakeSharedArray<uint8_t>(BytePerSec);
    // auto buff2 = MakeSharedArray<uint8_t>(BytePerSec);
    uint8_t buff1[512], buff2[512];

    int clusID1 = file1->FstClus;
    int clusID2 = file2->FstClus, newClus2;
    while (true) {
        int nextClus2 = get_fat_next_cluster(clusID2);
        if (nextClus2 >= 0xFF8 && nextClus2 <= 0xFFF) break;
        clusID2 = nextClus2;
    }
    read_sector(clusID2 + ClusToSec, buff2);
    int id = file2->FileSize % BytePerSec;
    if (id == 0) {
        clusID2 = append_a_cluster(clusID2);
        if (clusID2 == -1) return false;
        read_sector(clusID2 + ClusToSec, buff2);
    }

    while (true) {
        if (clusID1 >= 0xFF8 && clusID1 <= 0xFFF) break;

        read_sector(clusID1 + ClusToSec, buff1);
        for (int i = id; i < BytePerSec; i++)
            buff2[i] = buff1[i - id];
        write_sector(clusID2 + ClusToSec, buff2);

        if (id < BytePerSec - 1) {
            clusID2 = append_a_cluster(clusID2);
            if (clusID2 == -1) return false;
            read_sector(clusID2 + ClusToSec, buff2);
        }
        int st = BytePerSec - id;
        id = 0;
        for (int i = st; i < BytePerSec; i++)
            buff2[id++] = buff1[i];

        clusID1 = get_fat_next_cluster(clusID1);
    }
    write_sector(clusID2 + ClusToSec, buff2);
    file2->FileSize += file1->FileSize;
    return true;
}

void shell_help() {
    printf("%s\n", msg_help);
}

void shell_ls(struct File* nowDir) {
    char par[50];
    scanf("%s", par);
    if (strcmp(par, "-l") == 0)
        list_directory(nowDir, false);
    else if (strcmp(par, "-al") == 0)
        list_directory(nowDir, true);
    else {
        printf("usage: ls -l or ls -al\n");
    }
}

void shell_tree(struct File* nowDir) {
    int level;
    char par[50];
    scanf("%s%d", par, &level);
    if (strcmp(par, "-L") == 0)
        tree_directory(nowDir, false, level);
    else if (strcmp(par, "-aL") == 0)
        tree_directory(nowDir, true, level);
    else {
        printf("usage: tree -L level or tree -aL level\n");
        printf("Use help to see explanation.\n");
    }
}

void shell_cd(struct File** ptr_to_nowDir, char* nowPath) {
    char s[50] = {0}, dirPath[50] = {0}, nextPath[50] = {0};
    scanf("%s", s);
    standard_dir_path(dirPath, s);

    struct File* nowDir = *ptr_to_nowDir;
    struct File* nextDir = nowDir;
    strcpy(nextPath, nowPath);

    if (try_cd(dirPath, &nextDir, nextPath) == true) {
        nowDir = nextDir;
        strcpy(nowPath, nextPath);
        printf("Successfully changed directory.\n");
    } else
        printf("Failed to change directory.\n");
    *ptr_to_nowDir = nowDir;
}

char to_upper(const char c) {
    if (c >= 'a' && c <= 'z') return c - 'a' + 'A';
    return c;
}

void standard_dir_path(char* s, const char* dirPath) {
    if (strcmp(dirPath, "/") == 0) {
        strcpy(s, dirPath);
        return;
    }
    int len = 0;
    for (int i = 0; i < strlen(dirPath); i++) {
        if (i == strlen(dirPath) - 1 && dirPath[i] == '/') continue;
        s[len++] = to_upper(dirPath[i]);
    }
    s[len] = 0;
}

void divede_path(const char* dirPath, char* fstDir, char* resPath) {
    int fst_len = 0, res_len = 0;
    // fstDir = "";
    // resPath = "";
    int i;
    for (i = 0; i < strlen(dirPath); i++) {
        if (dirPath[i] == '/') break;
        fstDir[fst_len++] = dirPath[i];
    }
    for (i = i + 1; i < strlen(dirPath); i++) {
        resPath[res_len++] = dirPath[i];
    }
    fstDir[fst_len] = 0;
    resPath[res_len] = 0;
}

void fa_dir_path(char* fapath, const char* dirPath) {
    int len = 0;
    int i;
    for (i = strlen(dirPath) - 1; i >= 0; i--)
        if (dirPath[i] == '/') break;
    for (int j = 0; j < i; j++) fapath[len++] = dirPath[j];
    if (len == 0) fapath[len++] = '/';
    fapath[len] = 0;
}

bool try_cd(const char* dirPath, struct File** ptr_to_nowDir, char* nowPath) {
    struct File* nowDir = *ptr_to_nowDir;
    if (strlen(dirPath) == 0) return true;
    if (dirPath[0] == '/') {
        nowDir = &rootfile;

        strcpy(nowPath, "/");
        char newDirPath[50] = {0};
        int len = 0;
        // string newDirPath = "";
        for (int i = 1; i < strlen(dirPath); i++)
            newDirPath[len++] = dirPath[i];

        *ptr_to_nowDir = nowDir;
        return try_cd(newDirPath, &nowDir, nowPath);
    }

    char fstDir[50] = {0}, resPath[50] = {0};
    divede_path(dirPath, fstDir, resPath);

    char filename[50] = {0};
    generate_filename(fstDir, filename);

    nowDir = change_directory(nowDir, filename);  //generate_filename(fstDir));
    if (nowDir == NULL) return false;

    if (strcmp(fstDir, "..") == 0)
        fa_dir_path(nowPath, nowPath);
    else if (strcmp(fstDir, ".") != 0) {
        if (strcmp(nowPath, "/") != 0) strcat(nowPath, "/");
        // if (nowPath != "/") nowPath += "/";
        // nowPath += fstDir;
        strcat(nowPath, fstDir);
    }
    *ptr_to_nowDir = nowDir;
    return try_cd(resPath, ptr_to_nowDir, nowPath);
}

void file_path_to_dir_path(const char* filePath, char* dirPath, char* filename) {
    memset(dirPath, 0, sizeof(dirPath));
    memset(filename, 0, sizeof(filename));
    int i;
    for (i = strlen(filePath) - 1; i >= 0; i--) {
        if (filePath[i] == '/') break;
    }

    char s[50];
    int len = 0;

    for (int j = 0; j < i; j++)
        s[len++] = filePath[j];
    s[len] = 0;
    standard_dir_path(dirPath, s);

    len = 0;
    for (int j = i + 1; j < strlen(filePath); j++)
        s[len++] = filePath[j];

    s[len] = 0;
    generate_filename(s, filename);
}

void shell_touch(struct File* nowDir) {
    char filePath[50] = {0}, dirPath[50] = {0}, filename[50] = {0}, tmpPath[50] = {0};
    scanf("%s", filePath);
    file_path_to_dir_path(filePath, dirPath, filename);
    if (try_cd(dirPath, &nowDir, tmpPath) == false) {
        printf("Failed to create empty file.\n");
        return;
    }

    // time_t nowtime = time(0);
    // auto time = localtime(&nowtime);

    // if (create_empty_file(nowDir, filename, 0x20,
    //   generate_time(time->tm_hour, time->tm_min),
    //   generate_date(time->tm_year + 1900, time->tm_mon + 1, time->tm_mday)) == true) {
    if (create_empty_file(nowDir, filename, 0x20, generate_time(0, 0), generate_date(0 + 1900, 1, 0)) == true) {
        printf("Successfully created empty file.\n");
    } else {
        printf("Failed to create empty file.\n");
    }
}

void shell_mkdir(struct File* nowDir) {
    char filePath[50] = {0}, dirPath[50] = {0}, filename[50] = {0}, tmpPath[50] = {0};
    scanf("%s", filePath);
    file_path_to_dir_path(filePath, dirPath, filename);

    if (try_cd(dirPath, &nowDir, tmpPath) == false) {
        printf("Failed to create empty directory.\n");
        return;
    }

    // time_t nowtime = time(0);
    // auto time = localtime(&nowtime);
    // if (create_empty_directory(nowDir, filename,
    //    generate_time(time->tm_hour, time->tm_min),
    //    generate_date(time->tm_year + 1900, time->tm_mon + 1, time->tm_mday)) == true) {
    if (create_empty_directory(nowDir, filename, generate_time(0, 0), generate_date(0 + 1900, 1, 0)) == true) {
        printf("Successfully created empty directory.\n");
    } else
        printf("Failed to create empty directory.\n");
}

void shell_cat(struct File* nowDir) {
    char filePath[50] = {0}, dirPath[50] = {0}, filename[50] = {0}, tmpPath[50] = {0};
    scanf("%s", filePath);
    file_path_to_dir_path(filePath, dirPath, filename);

    if (try_cd(dirPath, &nowDir, tmpPath) == false) {
        printf("Failed to open and print the file.\n");
        return;
    }

    struct File* file = is_file_exist(nowDir, filename);
    if (file == NULL) {
        printf("Error: No such a file.\n");
        return;
    }
    if (file->Attr == 0x10) {
        printf("Error: It's a directory.\n");
        return;
    }

    struct ActiveFile* acfile = open_file(file, ReadOnly);
    if (acfile == NULL) {
        printf("The active file list is full. Failed to open and print the file.\n");
        return;
    }
    read_file(acfile);
    close_file(acfile);
    // printf("Successfully printed the content.\n");
}

void shell_write(struct File* nowDir, bool from_sda, uint32_t filesize) {
    char filePath[50] = {0}, dirPath[50] = {0}, filename[50] = {0}, tmpPath[50] = {0};
    scanf("%s", filePath);
    file_path_to_dir_path(filePath, dirPath, filename);
    if (try_cd(dirPath, &nowDir, tmpPath) == false) {
        printf("Failed to open and print the file.\n");
        return;
    }
    struct File* file = is_file_exist(nowDir, filename);
    if (file == NULL) {
        printf("Error: No such a file.\n");
        return;
    }
    if (file->Attr == 0x10) {
        printf("Error: It's a directory.\n");
        return;
    }
    struct ActiveFile* acfile = open_file(file, ReadWrite);
    if (acfile == NULL) {
        printf("The active file list is full. Failed to open and print the file.\n");
        return;
    }
    if ((from_sda == true && write_file_from_sda(acfile, filesize)) || (from_sda == false && write_file(acfile)))
        printf("Successfully wroted the file\n");
    else
        printf("Failed to write the file\n");
    close_file(acfile);
}

void shell_append(struct File* nowDir) {  // here
    struct File* nowDir1 = nowDir;
    struct File* nowDir2 = nowDir;

    char filePath1[50] = {0}, dirPath1[50] = {0}, filename1[50] = {0}, tmpPath1[50] = {0};
    char filePath2[50] = {0}, dirPath2[50] = {0}, filename2[50] = {0}, tmpPath2[50] = {0};
    scanf("%s%s", filePath1, filePath2);

    file_path_to_dir_path(filePath1, dirPath1, filename1);
    file_path_to_dir_path(filePath2, dirPath2, filename2);

    if (try_cd(dirPath1, &nowDir1, tmpPath1) == false || try_cd(dirPath2, &nowDir2, tmpPath2) == false) {
        printf("Failed to open and print the file.\n");
        return;
    }

    struct File* file1 = is_file_exist(nowDir1, filename1);
    struct File* file2 = is_file_exist(nowDir2, filename2);
    if (file1 == NULL || file2 == NULL) {
        printf("Error: Both files should exist.\n");
        return;
    }
    if (file1->Attr == 0x10 || file2->Attr == 0x10) {
        printf("Error: Can't append a directory\n");
        return;
    }
    struct ActiveFile* acfile1 = open_file(file1, ReadOnly);
    struct ActiveFile* acfile2 = open_file(file2, ReadWrite);
    // printf("in append: acfile1 = %s  acfile2 = %s\n", acfile1->file->Filename, acfile2->file->Filename);

    if (append_file(acfile1->file, acfile2->file))
        printf("Successfullly appended file.\n");
    else
        printf("Failed to append file. \n");
}

void shell_rm(struct File* nowDir) {
    // string s, filePath, dirPath, filename, tmpPath = "";
    // cin >> s;

    char s[10] = {0}, filePath[50] = {0}, dirPath[50] = {0}, filename[50] = {0}, tmpPath[50] = {0};
    scanf("%s", s);

    if (strcmp(s, "-r") == 0) {
        // cin >> filePath;
        scanf("%s", filePath);
        file_path_to_dir_path(filePath, dirPath, filename);
        if (try_cd(dirPath, &nowDir, tmpPath) == false) {
            printf("Failed to open and print the file.\n");
            return;
        };
        struct File* file = is_file_exist(nowDir, filename);
        if (file == NULL) {
            printf("Error: No such a file.\n");
            return;
        }
        if (delete_directory(nowDir, file, true))
            printf("Successfully deleted the directory recursively.\n");
        else
            printf("Failed to delete the directory recursively.\n");
    } else {
        // filePath = s;
        strcpy(filePath, s);
        file_path_to_dir_path(filePath, dirPath, filename);
        if (try_cd(dirPath, &nowDir, tmpPath) == false) {
            printf("Failed to open and print the file.\n");
            return;
        }
        struct File* file = is_file_exist(nowDir, filename);
        if (file == NULL) {
            printf("Error: No such a file.\n");
            return;
        }
        if (file->Attr == 0x10) {
            printf("Error: It's a directory.\n");
            return;
        }
        if (delete_file(nowDir, file))
            printf("Successfully deleted the file.\n");
        else
            printf("Failed to delete the file.\n");
    }
}

void shell_rmdir(struct File* nowDir) {
    char filePath[50] = {0}, dirPath[50] = {0}, filename[50] = {0}, tmpPath[50] = {0};
    scanf("%s", filePath);
    file_path_to_dir_path(filePath, dirPath, filename);
    if (try_cd(dirPath, &nowDir, tmpPath) == false) {
        printf("Failed to open and print the file.\n");
        return;
    };
    struct File* file = is_file_exist(nowDir, filename);
    if (delete_directory(nowDir, file, false))
        printf("Successfully deleted the directory.\n");
    else
        printf("Failed to delete the directory.\n");
}

void shell_exec(struct File* nowDir) {
    char filePath[50] = {0}, dirPath[50] = {0}, filename[50] = {0}, tmpPath[50] = {0};
    scanf("%s", filePath);
    file_path_to_dir_path(filePath, dirPath, filename);

    if (try_cd(dirPath, &nowDir, tmpPath) == false) {
        printf("Failed to open and print the file.\n");
        return;
    }

    struct File* file = is_file_exist(nowDir, filename);
    if (file == NULL) {
        printf("Error: No such a file.\n");
        return;
    }
    if (file->Attr == 0x10) {
        printf("Error: It's a directory.\n");
        return;
    }

    struct ActiveFile* acfile = open_file(file, ReadOnly);
    if (acfile == NULL) {
        printf("The active file list is full. Failed to open and print the file.\n");
        return;
    }
    int pid = fork();
    if (pid) {
        int32_t status;
        wait(pid, &status);
    } else {
        execv(acfile, NULL);
    }
    close_file(acfile);
    // printf("Successfully printed the content.\n");
}

void shell_write_program_to_sdb() {
    shell_touch(&rootfile);

    uint32_t filesize;
    scanf("%d", &filesize);
    shell_write(&rootfile, true, filesize);
}