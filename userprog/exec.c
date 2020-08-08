#include "exec.h"

#include "common.h"
#include "filesystem.h"
#include "global.h"
#include "memory.h"
#include "string.h"
#include "thread.h"
#include "stdio.h"
#include "ide.h"

extern void intr_exit();
typedef uint32_t Elf32_Word, Elf32_Addr, Elf32_Off;
typedef uint16_t Elf32_Half;

// 32位elf头 
struct Elf32_Ehdr {
    unsigned char e_ident[16];
    Elf32_Half e_type;
    Elf32_Half e_machine;
    Elf32_Word e_version;
    Elf32_Addr e_entry;
    Elf32_Off e_phoff;
    Elf32_Off e_shoff;
    Elf32_Word e_flags;
    Elf32_Half e_ehsize;
    Elf32_Half e_phentsize;
    Elf32_Half e_phnum;
    Elf32_Half e_shentsize;
    Elf32_Half e_shnum;
    Elf32_Half e_shstrndx;
};

// 程序头表Program header.就是段描述头 
struct Elf32_Phdr {
    Elf32_Word p_type;  // 见下面的enum segment_type
    Elf32_Off p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
};

// 段类型 
enum segment_type {
    PT_NULL,     // 忽略
    PT_LOAD,     // 可加载程序段
    PT_DYNAMIC,  // 动态加载信息
    PT_INTERP,   // 动态加载器名称
    PT_NOTE,     // 一些辅助信息
    PT_SHLIB,    // 保留
    PT_PHDR      // 程序头表
};

extern uint8_t img[512 * 20];

void sys_update_img() {
    uint8_t buff[512];
    for (int i = 0; i < 20; i++) {
        ide_read(sdb, i, buff, 1);
        // read_sector(i, buff);
        for (int j = 0; j < 512; j++) {
            img[i * 512 + j] = buff[j];
        }
    }
}

uint16_t sys_get_fat_next_cluster(const uint16_t clusID) {
    sys_update_img();
    int pos = clusID / 2 * 3 + clusID % 2;  // pos ~ pos + 1
    uint16_t nextClus = 0;
    if ((clusID % 2) == 0) {
        nextClus = (((uint16_t)(img[512 + pos + 1] & 0b00001111)) << 8) + (uint16_t)img[512 + pos];
    } else {
        nextClus = (((uint16_t)(img[512 + pos] & 0b11110000)) >> 4) + ((uint16_t)img[512 + pos + 1] << 4);
    }
    return nextClus;
}


/**
 * @brief 若成功则返回读出的字节数,到文件尾则返回-1 
 * 
 */
int32_t sys_read_file(struct ActiveFile* acfile, uint32_t offset, uint32_t size, void* _dst) {
    uint8_t* dst = _dst;
    uint16_t clusID = acfile->file->FstClus;
    uint8_t buff[512];
    int dst_len = 0;

    while (true) {
        if (clusID >= 0xFF8 && clusID <= 0xFFF) break;
        ide_read(sdb, clusID + ClusToSec, buff, 1);

        if (offset >= 512) {
            offset -= 512;
        } else {
            for (int i = 0 + offset; i < BytePerSec; i++) {
                dst[dst_len++] = buff[i];
                if (dst_len == size) return size;
            }
            offset = 0;
        }
        clusID = sys_get_fat_next_cluster(clusID);
    }

    return -1;
}

/**
 * @brief 将file指向的文件中,偏移为offset,大小为filesz的段加载到虚拟地址为vaddr的内存
 * 
 */
bool segment_load(struct ActiveFile* acfile, uint32_t offset, uint32_t filesz, uint32_t vaddr) {
    uint32_t vaddr_first_page = vaddr & 0xfffff000;                // vaddr地址所在的页框
    uint32_t size_in_first_page = PG_SIZE - (vaddr & 0x00000fff);  // 加载到内存后,文件在第一个页框中占用的字节大小
    uint32_t occupy_pages = 0;
    /* 若一个页框容不下该段 */
    if (filesz > size_in_first_page) {
        uint32_t left_size = filesz - size_in_first_page;
        occupy_pages = DIV_ROUND_UP(left_size, PG_SIZE) + 1;  // 1是指vaddr_first_page
    } else {
        occupy_pages = 1;
    }

    /* 为进程分配内存 */
    uint32_t page_idx = 0;
    uint32_t vaddr_page = vaddr_first_page;
    while (page_idx < occupy_pages) {
        uint32_t* pde = pde_ptr(vaddr_page);
        uint32_t* pte = pte_ptr(vaddr_page);

        /* 如果pde不存在,或者pte不存在就分配内存.
       * pde的判断要在pte之前,否则pde若不存在会导致
       * 判断pte时缺页异常 */
        if (!(*pde & 0x00000001) || !(*pte & 0x00000001)) {
            if (get_one_page(PF_USER, vaddr_page) == NULL) {
                return false;
            }
        }  // 如果原进程的页表已经分配了,利用现有的物理页,直接覆盖进程体
        vaddr_page += PG_SIZE;
        page_idx++;
    }
    sys_read_file(acfile, offset, filesz, (void*)vaddr);
    return true;
}


/**
 * @brief 从文件系统上加载用户程序,成功则返回程序的起始地址,否则返回-1
 * 
 */
int32_t load(struct ActiveFile* acfile) {
    int32_t ret = -1;
    struct Elf32_Ehdr elf_header;
    struct Elf32_Phdr prog_header;
    memset(&elf_header, 0, sizeof(struct Elf32_Ehdr));

    if (sys_read_file(acfile, 0, sizeof(struct Elf32_Ehdr), &elf_header) != sizeof(struct Elf32_Ehdr)) {
        return -1;
    }

    /* 校验elf头 */
    if (memcmp(elf_header.e_ident, "\177ELF\1\1\1", 7) || elf_header.e_type != 2 || elf_header.e_machine != 3 || elf_header.e_version != 1 || elf_header.e_phnum > 1024 || elf_header.e_phentsize != sizeof(struct Elf32_Phdr)) {
        return -1;
    }

    Elf32_Off prog_header_offset = elf_header.e_phoff;
    Elf32_Half prog_header_size = elf_header.e_phentsize;

    /* 遍历所有程序头 */
    uint32_t prog_idx = 0;
    while (prog_idx < elf_header.e_phnum) {
        memset(&prog_header, 0, prog_header_size);
        if (sys_read_file(acfile, prog_header_offset, prog_header_size, &prog_header) != prog_header_size) {
            return -1;
        }

        /* 如果是可加载段就调用segment_load加载到内存 */
        if (PT_LOAD == prog_header.p_type) {
            if (!segment_load(acfile, prog_header.p_offset, prog_header.p_filesz, prog_header.p_vaddr)) {
                return -1;
            }
        }

        /* 更新下一个程序头的偏移 */
        prog_header_offset += elf_header.e_phentsize;
        prog_idx++;
    }
    return elf_header.e_entry;
    // ret = elf_header.e_entry;
    // return ret;
}

/**
 * @brief 用acfile指向的程序替换当前进程
 * 
 */
int32_t sys_execv(struct ActiveFile* acfile, const char* argv[]) {
    uint32_t argc = 0;

    int32_t entry_point = load(acfile);
    if (entry_point == -1) {  // 若加载失败则返回-1
        return -1;
    }

    struct pcb* cur = running_thread();
    /* 修改进程名 */
    memcpy(cur->name, acfile->file->Filename, 16);

    /* 修改栈中参数 */
    struct intr_stack* intr_0_stack = (struct intr_stack*)((uint32_t)cur + PG_SIZE - sizeof(struct intr_stack));
    /* 参数传递给用户进程 */
    intr_0_stack->ebx = (int32_t)argv;
    intr_0_stack->ecx = argc;
    intr_0_stack->eip = (void*)entry_point;
    /* 使新用户进程的栈地址为最高用户空间地址 */
    intr_0_stack->esp = (void*)0xc0000000;

    //直接从中断返回 
    asm volatile("movl %0, %%esp; jmp intr_exit"
                 :
                 : "g"(intr_0_stack)
                 : "memory");
    return 0;
}
