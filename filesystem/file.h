#pragma once

#include "common.h"
#include "global.h"

void file_init();

void init_new_file(struct File *file);

struct File *get_empty_file();

void init_file(struct File *file, struct File *fa, char *filename, uint8_t attr, uint16_t wrttime, uint16_t wrtdate, uint16_t fstclus, uint32_t filesize);

struct File *make_file_from_sector(struct File *fa, uint8_t *buff, uint32_t st);

void file_print_all(struct File *file, bool ShowHid);

void file_print_filesize(struct File *file);

void file_print_name(struct File *file);

void file_print_attr(struct File *file);

void file_print_time(struct File *file);

void file_print_date(struct File *file);

uint16_t generate_date(int32_t year, int32_t month, int32_t day);

uint16_t generate_time(int32_t hour, int32_t minute);

void generate_filename(char *name, char *filename);

bool create_dir_entry(struct File *dir, struct File *file);

bool write_empty_directory(struct File *fa, struct File *dir);

struct AFBLNode *get_empty_AFBLNode();

void init_AFBLNode(struct AFBLNode *node, uint8_t *writeBuff, struct AFBLNode *next);

void active_file_open(struct ActiveFile *acfile, struct File *file, enum FileOpenType Type);

void active_file_close(struct ActiveFile *acfile);

void clear_file(struct File *file);

struct ActiveFile *open_file(struct File *file, enum FileOpenType Type);

void close_file(struct ActiveFile *acfile);