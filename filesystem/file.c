#include "file.h"

// #include <stdio.h>

#include "common.h"
#include "disk.h"
#include "string.h"
#include "stdio.h"
#include "syscall.h"

struct File rootfile;
struct File filelist[MAX_FILE_CNT];
int active_cnt;
struct ActiveFile activelist[MAX_ACTIVE_CNT];
struct AFBLNode AFBLNodelist[MAX_NODE_CNT];

void file_init() {
    init_new_file(&rootfile);
    for (int i = 0; i < MAX_FILE_CNT; i++) {
        filelist[i].used = false;
    }
    active_cnt = 0;
    for (int i = 0; i < MAX_NODE_CNT; i++) {
        AFBLNodelist[i].used = false;
    }
    for (int i = 0; i < MAX_ACTIVE_CNT; i++) {
        activelist[i].used = false;
    }
}

void init_new_file(struct File *file) {
    if (file == NULL) return;
    file->used = true;
    memset(file->Filename, 0, sizeof(file->Filename));
    // for (int32_t i = 0; i < 11; i++)
    // file->Filename[i] = '@';
    file->Fa = NULL;
    file->Attr = 0;
    file->WrtTime = 0;
    file->WrtDate = 0;
    file->FstClus = 0;
    file->FileSize = 0;
    file->FstSon = NULL;
    file->NextSib = NULL;
}

struct File *get_empty_file() {
    for (int32_t i = 0; i < MAX_FILE_CNT; i++) {
        if (filelist[i].used == false) {
            init_new_file(&filelist[i]);
            return &filelist[i];
        }
    }
    return NULL;
}

void init_file(struct File *file, struct File *fa, char *filename, uint8_t attr, uint16_t wrttime, uint16_t wrtdate, uint16_t fstclus, uint32_t filesize) {
    file->Fa = fa;
    for (int i = 0; i < 11; i++)
        file->Filename[i] = filename[i];
    file->Attr = attr;
    file->WrtTime = wrttime;
    file->WrtDate = wrtdate;
    file->FstClus = fstclus;
    file->FileSize = filesize;
    file->FstSon = NULL;
    file->NextSib = NULL;
}

struct File *make_file_from_sector(struct File *fa, uint8_t *buff, uint32_t st) {
    struct File *file = get_empty_file();
    if (file == NULL) return NULL;

    file->Fa = fa;
    for (int32_t i = 0; i < 11; i++)
        file->Filename[i] = buff[st + i];

    file->Attr = buff[st + 11];
    for (int32_t i = 0; i < 10; i++) {
        file->Reserved[i] = buff[st + 12 + i];
    }
    file->WrtTime = get_u16(buff, st + 22);
    file->WrtDate = get_u16(buff, st + 24);
    file->FstClus = get_u16(buff, st + 26);
    file->FileSize = get_u32(buff, st + 28);
    return file;
    // printf("file name = %s\n", file->Filename);
}

void file_print_all(struct File *file, bool ShowHid) {
    // ASSERT(file != NULL);
    if (file == NULL) return;
    if (!ShowHid) {
        if (file->Attr == 0x02 || file->Filename[0] == 0x2E) return;
    }
    printf("Filename: ");
    file_print_name(file);

    printf("    Attribute: ");
    file_print_attr(file);

    printf("    Last Write Time: ");
    file_print_time(file);

    printf("    Last Write Date: ");
    file_print_date(file);

    printf("    File Size: ");
    file_print_filesize(file);

    printf("\n");
}

void file_print_filesize(struct File *file) {
    printf("%d", file->FileSize);
}

void file_print_name(struct File *file) {
    int32_t cnt = 0;
    for (int32_t i = 0; i < 8; i++)
        if (file->Filename[i] != 0x20) {  // no space
            printf("%c", file->Filename[i]);
            cnt++;
        }
    if (file->Filename[8] != 0x20) {
        printf(".");
        cnt++;
    }
    for (int32_t i = 8; i < 11; i++)
        if (file->Filename[i] != 0x20) {
            printf("%c", file->Filename[i]);
            cnt++;
        }
    for (int32_t i = cnt; i < 12; i++)
        printf(" ");
}

void file_print_attr(struct File *file) {
    switch (file->Attr) {
        case 0x01:
            printf("read-only   ");
            break;
        case 0x02:
            printf("hidden      ");
            break;
        case 0x04:
            printf("system      ");
            break;
        case 0x08:
            printf("volume label");
            break;
        case 0x10:
            printf("subdirectory");
            break;
        case 0x20:
            printf("archive     ");
            break;
        default:
            printf("unused      ");
    }
}

void file_print_time(struct File *file) {
    int32_t x = file->WrtTime;

    int32_t hour = x & 0b1111100000000000;
    hour = hour >> 11;
    int32_t minute = x & 0b0000011111100000;
    minute = minute >> 5;

    printf("%d%d:%d%d", (hour / 10) % 10, hour % 10, (minute / 10) % 10, minute % 10);
}

void file_print_date(struct File *file) {
    int32_t x = file->WrtDate;

    int32_t year = x & 0b1111111000000000;
    year = year >> 9;
    int32_t month = x & 0b0000000111100000;
    month = month >> 5;
    int32_t day = x & 0b0000000000011111;
    year += 1980;
    printf("%d%d-%d%d-%d%d", (month / 10) % 10, month % 10, (day / 10) % 10, day % 10, (year / 10) % 10, year % 10);
}

uint16_t generate_date(int32_t year, int32_t month, int32_t day) {
    uint16_t date = ((year - 1980) << 9) + (month << 5) + day;
    return date;
}

uint16_t generate_time(int32_t hour, int32_t minute) {
    uint16_t time = (hour << 11) + (minute << 5);
    return time;
}

void generate_filename(char *name, char *filename) {
    if (strcmp(name, ".") == 0) {
        strcpy(filename, ".          ");
        return;
    }
    if (strcmp(name, "..") == 0) {
        strcpy(filename, "..         ");
        return;
    }
    char fst[30] = {0}, ext[30] = {0};
    int filename_len = 0, fst_len = 0, ext_len = 0;
    bool dot = false;
    for (int i = 0; i < strlen(name); i++) {
        if (name[i] == '.') {
            dot = true;
            continue;
        }
        if (name[i] >= 'a' && name[i] <= 'z') name[i] = name[i] - 'a' + 'A';
        if (dot == false)
            fst[fst_len++] = name[i];
        else
            ext[ext_len++] = name[i];
    }
    for (int i = 0; i < min(fst_len, 8); i++) 
        filename[filename_len++] = fst[i];
    while (filename_len < 8)
        filename[filename_len++] = ' ';
    for (int i = 0; i < min(ext_len, 3); i++)
        filename[filename_len++] = ext[i];
    while (filename_len < 11) 
        filename[filename_len++] = ' ';
    filename[filename_len] = '\0';
}

bool create_dir_entry(struct File *dir, struct File *file) {
    uint16_t clusID = (dir == &rootfile) ? 0 : dir->FstClus;
    uint16_t secID = (dir == &rootfile) ? 19 : clusID + ClusToSec;

    while (true) {
        if (clusID == 0 && secID == 33) break;
        if (clusID != 0 && clusID >= 0xFF8 && clusID <= 0xFFF) break;

        if (create_dir_entry_in_sector(secID, file)) return true;

        if (clusID == 0)
            secID++;
        else {
            clusID = get_fat_next_cluster(clusID);
            secID = clusID + ClusToSec;
        }
    }
    if (clusID == 0) return false;  // root directory has ran out

    uint16_t newClus = append_a_cluster(clusID);
    return create_dir_entry_in_sector(newClus + ClusToSec, file);
}

bool write_empty_directory(struct File *fa, struct File *dir) {
    struct File *dot = get_empty_file();
    init_file(dot, dir, ".          ", dir->Attr, dir->WrtTime, dir->WrtDate, dir->FstClus, dir->FileSize);
    if (!create_dir_entry(dir, dot)) {
        dot->used = false;
        return false;
    }
    struct File *dotdot = get_empty_file();
    init_file(dotdot, dir, "..         ", fa->Attr, fa->WrtTime, fa->WrtDate, fa->FstClus, fa->FileSize);
    if (!create_dir_entry(dir, dotdot)) {
        dotdot->used = false;
        return false;
    }
    dir->FstSon = dot;
    dot->NextSib = dotdot;
    return true;
}

void init_new_AFBLNode(struct AFBLNode *node) {
    memset(node->buff, 0, sizeof(node->buff));
    node->next = NULL;
    node->used = true;
}

struct AFBLNode *get_empty_AFBLNode() {
    for (int i = 0; i < MAX_NODE_CNT; i++) {
        if (AFBLNodelist[i].used == false) {
            init_new_AFBLNode(&AFBLNodelist[i]);
            return &AFBLNodelist[i];
        }
    }
    return NULL;
}

void init_AFBLNode(struct AFBLNode *node, uint8_t *writeBuff, struct AFBLNode *next) {
    node->next = next;
    for (int i = 0; i < BytePerSec; i++)
        node->buff[i] = writeBuff[i];
}

void active_file_open(struct ActiveFile *acfile, struct File *file, enum FileOpenType Type) {
    active_cnt++;
    acfile->file = file;
    acfile->RefCnt = 1;
    acfile->Type = Type;
    acfile->head = NULL;
    acfile->buffsize = 0;
}

void active_file_close(struct ActiveFile *acfile) {
    active_cnt--;

    acfile->file->FileSize = acfile->buffsize;
    uint16_t clusID = acfile->file->FstClus;
    struct AFBLNode *now = acfile->head, *pre = NULL;
    while (now != NULL) {
        write_sector(clusID + ClusToSec, now->buff);
        pre = now;
        now = now->next;
        pre->used = false;
        clusID = get_fat_next_cluster(clusID);
    }
    acfile->file = NULL;
    acfile->RefCnt = 0;
    acfile->Type = ReadOnly;
    acfile->head = NULL;
    acfile->buffsize = 0;
}

void clear_file(struct File *file) {
    int clusID = file->FstClus, nextClus = get_fat_next_cluster(clusID);
    while (true) {
        if (clusID >= 0xFF8 && clusID <= 0xFFF) break;

        clear_cluster(clusID);
        write_fat_next_cluster(clusID, 0x000);
        clusID = nextClus;
        nextClus = get_fat_next_cluster(clusID);
    }
}

struct ActiveFile *open_file(struct File *file, enum FileOpenType Type) {
    for (int i = 0; i < MAX_ACTIVE_CNT; i++) {
        if (activelist[i].file == file && activelist[i].Type == Type) {
            activelist[i].RefCnt++;
            return &activelist[i];
        }
    }
    for (int i = 0; i < MAX_ACTIVE_CNT; i++) {
        if (activelist[i].RefCnt == 0) {
            active_file_open(&activelist[i], file, Type);
            return &activelist[i];
        }
    }
    return NULL;
}

void close_file(struct ActiveFile *acfile) {
    acfile->RefCnt--;
    if (acfile->RefCnt == 0)
        active_file_close(acfile);
}
