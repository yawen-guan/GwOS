#include "disk.h"

#include "common.h"
#include "debug.h"
#include "disk.h"
#include "stdio.h"
#include "string.h"
#include "syscall.h"
// #include "debug.h"
// #include "idx.h"
// #include <stdio.h>

uint8_t img[512 * 20];  //MBR + FAT1 + FAT2
// uint8_t img[ImgByteSize];

struct BootSector bootsec;

void disk_close() {
    for (int i = 0; i < 20; i++) {
        write_sector(i, img + i * 512);
    }
}

void update_img() {
    uint8_t buff[512];
    for (int i = 0; i < 20; i++) {
        read_sector(i, buff);
        for (int j = 0; j < 512; j++) {
            img[i * 512 + j] = buff[j];
        }
    }
}

bool boot_sector_init() {
    uint8_t buff[512];
    read_sector(0, buff);
    if (buff[510] != 0x55 || buff[511] != 0xAA) {
        return false;
    }
    for (int i = 3; i < 11; i++) bootsec.BS_OEMName[i - 3] = buff[i];
    bootsec.BPB_BytsPerSec = get_u16(buff, 11);
    bootsec.BPB_SecPerClus = get_u8(buff, 13);
    bootsec.BPB_ResvdSecCnt = get_u16(buff, 14);
    bootsec.BPB_NumFATs = get_u8(buff, 16);
    bootsec.BPB_RootEntCnt = get_u16(buff, 17);
    bootsec.BPB_TotSec16 = get_u16(buff, 19);
    bootsec.BPB_Media = get_u8(buff, 21);
    bootsec.BPB_FATSz16 = get_u16(buff, 22);
    bootsec.BPB_SecPerTrk = get_u16(buff, 24);
    bootsec.BPB_NumHeads = get_u16(buff, 26);
    bootsec.BPB_HiddSec = get_u32(buff, 28);
    bootsec.BPB_TotSec32 = get_u32(buff, 32);
    bootsec.BPB_DrvNum = get_u8(buff, 36);
    bootsec.BS_Reserved1 = get_u8(buff, 37);
    bootsec.BS_BootSig = get_u8(buff, 38);
    bootsec.BS_VolID = get_u32(buff, 39);
    for (int i = 43; i < 54; i++) bootsec.BS_VolLab[i - 43] = buff[i];
    for (int i = 54; i < 62; i++) bootsec.BS_FileSysType[i - 54] = buff[i];
    if (bootsec.BPB_TotSec32 != 0) return false;
    return true;
}

void show_boot_sector_info() {
    printf("Bytes per sector:  %d\n", (int)bootsec.BPB_BytsPerSec);
    printf("Sectors per cluster:  %d\n", (int)bootsec.BPB_SecPerClus);
    printf("Number of reserved sectors:  %d\n", (int)bootsec.BPB_ResvdSecCnt);
    printf("Number of FATs: %d\n", (int)bootsec.BPB_NumFATs);
    printf("Maximum number of root directory entries: %d\n", (int)bootsec.BPB_RootEntCnt);
    printf("Total sector count(16): %d\n", (int)bootsec.BPB_TotSec16);
    printf("Media: %d\n", (int)bootsec.BPB_Media);
    printf("Sectors per FAT: %d\n", (int)bootsec.BPB_FATSz16);
    printf("Sectors per track: %d\n", (int)bootsec.BPB_SecPerTrk);
    printf("Number of heads: %d\n", (int)bootsec.BPB_NumHeads);
    printf("Number of hidden sectors: %d\n", (int)bootsec.BPB_HiddSec);
    printf("Total sector count for FAT32 (0 for FAT12 and FAT16): %d\n", (int)bootsec.BPB_TotSec32);
    printf("DrvNum: %d\n", (int)bootsec.BPB_DrvNum);
    printf("Boot signature: %d\n", (int)bootsec.BS_BootSig);
    printf("Volume id: %d\n", (int)bootsec.BS_VolID);
    printf("--End of Boot Sector Information.--\n\n");
}

int32_t min(int32_t x, int32_t y) {
    return (x < y) ? x : y;
}

uint8_t get_u8(const uint8_t* buff, int32_t st) {
    return buff[st];
}

uint16_t get_u16(const uint8_t* buff, int32_t st) {
    return (uint16_t)(buff[st]) + ((uint16_t)buff[st + 1] << 8);
}

uint32_t get_u32(const uint8_t* buff, int32_t st) {
    return (uint32_t)(buff[st]) + ((uint32_t)buff[st + 1] << 8) + ((uint32_t)buff[st + 2] << 16) + ((uint32_t)buff[st + 3] << 24);
}

void write_u8(uint8_t* buff, int st, uint8_t data) {
    buff[st] = data;
}

void write_u16(uint8_t* buff, int st, uint16_t data) {
    buff[st] = (uint8_t)data;
    buff[st + 1] = (uint8_t)(data >> 8);
}

void write_u32(uint8_t* buff, int st, uint32_t data) {
    buff[st] = (uint8_t)data;
    buff[st + 1] = (uint8_t)(data >> 8);
    buff[st + 2] = (uint8_t)(data >> 16);
    buff[st + 3] = (uint8_t)(data >> 24);
}

void read_sector(const uint16_t secID, uint8_t* buff) {
    ASSERT(secID >= 0 && secID <= SecCnt);
    read_disk(buff, secID, true);
    // for (int i = secID * 512; i < (secID + 1) * 512; i++) {
    // buff[i - secID * 512] = img[i];
    // }
}

void write_sector(const uint16_t secID, const uint8_t* buff) {
    ASSERT(secID >= 0 && secID <= SecCnt);
    write_disk(buff, secID, true);
    // for (int i = 0; i < 512; i++)
    // img[i + secID * 512] = buff[i];
}

void clear_cluster(const uint16_t clusID) {
    uint8_t buff[512];
    memset(buff, 0, sizeof(buff));
    write_sector(clusID + ClusToSec, buff);
}

void write_fat_sector(const uint16_t secID, const uint8_t* buff) {
    write_sector(secID, buff);
    write_sector(secID + 9, buff);
}

uint16_t get_fat_next_cluster(const uint16_t clusID) {
    update_img();
    int pos = clusID / 2 * 3 + clusID % 2;  // pos ~ pos + 1
    uint16_t nextClus = 0;
    if ((clusID % 2) == 0) {
        nextClus = (((uint16_t)(img[512 + pos + 1] & 0b00001111)) << 8) + (uint16_t)img[512 + pos];
    } else {
        nextClus = (((uint16_t)(img[512 + pos] & 0b11110000)) >> 4) + ((uint16_t)img[512 + pos + 1] << 4);
    }
    return nextClus;
}

void write_fat_next_cluster(const uint16_t clusID, const uint16_t data) {
    int pos = clusID / 2 * 3 + clusID % 2;  // pos ~ pos + 1
    uint8_t buff[512];
    read_sector(1 + pos / 512, buff);
    int secpos = pos % 512;
    if ((clusID % 2) == 0)
        buff[secpos] = (uint8_t)data;
    else {
        uint8_t x = buff[secpos] & 0b00001111;
        uint8_t y = (uint8_t)(data & 0b000000001111);
        buff[secpos] = x + (y << 4);
    }

    if (1 + pos / 512 != 1 + (pos + 1) / 512) {
        write_fat_sector(1 + pos / 512, buff);
        read_sector(1 + (pos + 1) / 512, buff);
    }
    secpos = (pos + 1) % 512;
    if ((clusID % 2) == 0) {
        uint8_t x = buff[secpos] & 0b11110000;
        uint8_t y = (uint8_t)(data >> 8) & 0b00001111;
        buff[secpos] = x + y;
    } else
        buff[secpos] = (uint8_t)(data >> 4);
    write_fat_sector(1 + (pos + 1) / 512, buff);
}

uint16_t find_empty_cluster() {
    update_img();
    uint8_t buff[512];
    uint16_t clusID = -1;
    for (int i = 512; i < 512 + BytePerSec * 9; i += 3) {  // i, i + 1, i + 2
        uint16_t x = ((uint16_t)(img[i + 1] & 0b00001111) << 8) + (uint16_t)img[i];
        if (x == 0x00) {
            clusID = (i - 512) / 3 * 2;
            break;
        }
        uint16_t y = ((uint16_t)(img[i + 1] & 0b11110000) >> 4) + ((uint16_t)img[i + 2] << 4);
        if (y == 0x00) {
            clusID = (i - 512) / 3 * 2 + 1;
            break;
        }
    }
    clear_cluster(clusID);
    return clusID;
}

uint16_t append_a_cluster(const uint16_t clusID) {
    uint16_t newClus = find_empty_cluster();
    if (newClus == -1) return -1;
    write_fat_next_cluster(clusID, newClus);
    write_fat_next_cluster(newClus, 0xFFF);
    return newClus;
}

bool create_dir_entry_in_sector(const uint16_t secID, struct File* file) {
    uint8_t buff[512];
    read_sector(secID, buff);

    for (int i = 0; i < BytePerSec; i += BytePerDirEntry) {
        if (buff[i + 0] == 0x00 || buff[i + 0] == 0xE5) {
            for (int j = 0; j < 11; j++) {
                write_u8(buff, i + j, file->Filename[j]);
            }
            write_u8(buff, i + 11, file->Attr);
            write_u16(buff, i + 22, file->WrtTime);
            write_u16(buff, i + 24, file->WrtDate);
            write_u16(buff, i + 26, file->FstClus);
            write_u32(buff, i + 28, file->FileSize);
            write_sector(secID, buff);
            return true;
        }
    }
    return false;
}
