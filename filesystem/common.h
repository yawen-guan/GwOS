#pragma once

#include "global.h"
#include "stdint.h"

#define SecCnt 2880
#define BytePerSec 512
#define ImgByteSize 1474560  // BytePerSec * SecCnt;
#define BitPerByte 8
#define BytePerDirEntry 32
#define ClusToSec 31
#define ClusSt 2
#define ClusEd 2849
#define MAX_FILE_CNT 100
#define MAX_NODE_CNT 100
#define MAX_ACTIVE_CNT 10

struct BootSector {
    char BS_OEMName[8];
    uint16_t BPB_BytsPerSec;
    uint8_t BPB_SecPerClus;
    uint16_t BPB_ResvdSecCnt;
    uint8_t BPB_NumFATs;
    uint16_t BPB_RootEntCnt;
    uint16_t BPB_TotSec16;
    uint8_t BPB_Media;
    uint16_t BPB_FATSz16;
    uint16_t BPB_SecPerTrk;
    uint16_t BPB_NumHeads;
    uint32_t BPB_HiddSec;
    uint32_t BPB_TotSec32;
    uint8_t BPB_DrvNum;
    uint8_t BS_Reserved1;
    uint8_t BS_BootSig;
    uint32_t BS_VolID;
    char BS_VolLab[11];
    char BS_FileSysType[8];
};

//The node of the filesystem tree. It can b e a normal file or a directory.
struct File {
    char Filename[11];
    uint8_t Attr;
    char Reserved[10];
    uint16_t WrtTime;
    uint16_t WrtDate;
    uint16_t FstClus;
    uint32_t FileSize;

    struct File *Fa;
    struct File *FstSon;
    struct File *NextSib;

    bool used;
};

enum FileOpenType {
    ReadOnly = 0,
    WriteOnly,
    ReadWrite
};

// Active File Buff List Node
struct AFBLNode {
    uint8_t buff[512];
    struct AFBLNode *next;

    bool used;
};

struct ActiveFile {
    struct File *file;
    int RefCnt;
    enum FileOpenType Type;
    struct AFBLNode *head;
    uint32_t buffsize;

    bool used;
};

extern struct File rootfile;
extern struct File filelist[MAX_FILE_CNT];
extern int active_cnt;
extern struct ActiveFile activelist[MAX_ACTIVE_CNT];
extern struct AFBLNode AFBLNodelist[MAX_NODE_CNT];