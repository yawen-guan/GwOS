#include "tss.h"

#include "global.h"
#include "print.h"
#include "stdint.h"
#include "string.h"

struct tss {
    uint32_t backlink;
    uint32_t* esp0;
    uint32_t ss0;
    uint32_t* esp1;
    uint32_t ss1;
    uint32_t* esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t (*eip)(void);
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint32_t trace;
    uint32_t io_base;
};

static struct tss tss;

/**
 * @brief 更新TSS中的esp0
 * 只修改0特权级对应的栈, 也就是thread的PCB所在页的最顶端
 * 
 * @param thread 
 */
void update_tss_esp(struct pcb* thread) {
    tss.esp0 = (uint32_t*)((uint32_t)thread + PG_SIZE);
}

/**
 * @brief 创建新的gdt描述符
 * 
 * @param desc_addr 
 * @param limit 
 * @param attr_low 
 * @param attr_high 
 * @return struct gdt_desc 
 */
struct gdt_desc make_gdt_desc(uint32_t* desc_addr, uint32_t limit, uint8_t attr_low, uint8_t attr_high) {
    debug_printf_s("make_gdt_desc ", "begin");

    uint32_t desc_base = (uint32_t)desc_addr;
    struct gdt_desc desc;

    debug_printf_s("make_gdt_desc ", "ok 1");

    desc.limit_low_word = limit & 0x0000ffff;
    desc.base_low_word = desc_base & 0x0000ffff;
    desc.base_mid_byte = ((desc_base & 0x00ff0000) >> 16);
    desc.attr_low_byte = (uint8_t)(attr_low);
    desc.limit_high_attr_high = (((limit & 0x000f0000) >> 16) + (uint8_t)(attr_high));
    desc.base_high_byte = desc_base >> 24;

    debug_printf_s("make_gdt_desc ", "finish");

    return desc;
}

/**
 * @brief 初始化tss并将其安装到gdt中，另外在GDT中安装DPL=3的数据段和代码段描述符，供用户进程使用
 * 
 */
void tss_init() {
    put_str("tss_init start\n", 0x07);

    uint32_t tss_size = sizeof(tss);
    memset(&tss, 0, sizeof(tss));
    tss.ss0 = SELECTOR_KERNEL_STACK;
    tss.io_base = tss_size;

    debug_printf_s("ok", "1");

    //gdt的段基址为0x900,一个描述符8B，tss为第四个描述符
    struct gdt_desc tss_desc = make_gdt_desc((uint32_t*)&tss, tss_size - 1, TSS_ATTR_LOW, TSS_ATTR_HIGH);

    debug_printf_s("ok", "1.5");

    *((uint32_t*)0xc0000920) = 1;

    debug_printf_s("ok", "1.8");

    *((struct gdt_desc*)0xc0000920) = tss_desc;

    debug_printf_s("ok", "2");

    //DPL=3的数据段和代码段描述符
    *((struct gdt_desc*)0xc0000928) = make_gdt_desc((uint32_t*)0, 0xfffff, GDT_CODE_ATTR_LOW_DPL3, GDT_ATTR_HIGH);
    *((struct gdt_desc*)0xc0000930) = make_gdt_desc((uint32_t*)0, 0xfffff, GDT_DATA_ATTR_LOW_DPL3, GDT_ATTR_HIGH);

    debug_printf_s("ok", "3");
    // 用指令lgdt加载gdt
    uint64_t gdt_operand = ((8 * 7 - 1) | ((uint64_t)(uint32_t)0xc0000900 << 16));  // 7个描述符大小
    asm volatile("lgdt %0"
                 :
                 : "m"(gdt_operand));

    debug_printf_s("ok", "4");
    // 用指令ltr加载tss到TR寄存器
    asm volatile("ltr %w0"
                 :
                 : "r"(SELECTOR_TSS));

    put_str("tss_init and ltr done\n", 0x07);
}