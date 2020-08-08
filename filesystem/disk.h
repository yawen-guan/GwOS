#pragma once

#include "global.h"
#include "stdint.h"
#include "common.h"
#include "ide.h"

// extern struct disk disk0, disk1;

// bool disk_init(const char* ImgName);

// bool disk_close(const char* ImgName);

// void disk_init();

void disk_close();

bool boot_sector_init();

void show_boot_sector_info();

int32_t min(int32_t x, int32_t y);

uint8_t get_u8(const uint8_t* buff, int32_t st);

uint16_t get_u16(const uint8_t* buff, int32_t st);

uint32_t get_u32(const uint8_t* buff, int32_t st);

void write_u8(uint8_t* buff, int st, uint8_t data);

void write_u16(uint8_t* buff, int st, uint16_t data);

void write_u32(uint8_t* buff, int st, uint32_t data);

void read_sector(const uint16_t secID, uint8_t* buff);

void write_sector(const uint16_t secID, const uint8_t* buff);

void clear_cluster(const uint16_t clusID);

void write_fat_sector(const uint16_t secID, const uint8_t* buff);

uint16_t get_fat_next_cluster(const uint16_t clusID);

void write_fat_next_cluster(const uint16_t clusID, const uint16_t data);

uint16_t find_empty_cluster();

uint16_t append_a_cluster(const uint16_t clusID);

bool create_dir_entry_in_sector(const uint16_t secID, struct File* file);