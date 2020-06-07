#include "stdint.h"
#include "print.h"
#include "interrupt.h"
#include "global.h"
#include "io.h"

// pic: programable interrupt controller, 8259A here
#define PIC_M_CTRL 0x20 // master片控制端口
#define PIC_M_DATA 0x21 // master片数据端口
#define PIC_S_CTRL 0xa0 // slave片控制端口
#define PIC_S_DATA 0xa1 // slave片数据端口

#define IDT_DESC_CNT 0x30 // 目前支持的中断数

#define EFLAGS_IF 0x00000200

// 门描述符
struct gate_desc{
    uint16_t func_offset_low_16bit;
    uint16_t selector; // 代码段描述符选择子
    uint8_t dcount; // 双字计数字段
    uint8_t attribute;
    uint16_t func_offset_high_16bit;
};

static struct gate_desc idt[IDT_DESC_CNT]; // interrupt descriptor table
extern void* intr_entry_table[IDT_DESC_CNT]; // 存储中断处理程序的调用入口
void* intr_function_table[IDT_DESC_CNT]; // 具体的中断处理程序
char* intr_name[IDT_DESC_CNT]; // 中断对应的异常的名字


/**
 * @brief 中断描述符表idt的初始化
*/ 
void idt_init(){
    for(int i = 0; i < IDT_DESC_CNT; i ++){
        idt[i].func_offset_low_16bit = (uint32_t)intr_entry_table[i] & 0x0000FFFF;
        idt[i].selector = SELECTOR_KERNEL_CODE;
        idt[i].dcount = 0;
        idt[i].attribute = IDT_DESC_ATTR_DPL0;
        idt[i].func_offset_high_16bit = (uint32_t)intr_entry_table[i] & 0xFFFF0000;
    }
    put_str("\nidt_init done\n", 0x07);
}

/**
 * @brief 通用中断处理函数
 */ 
static void general_intr_handler(uint8_t vector_id){
    if(vector_id == 0x27 || vector_id == 0x2f){ // IRQ7和IRQ15 伪中断
        return;
    }
    put_str("int vector: 0x", 0x07);
    put_int(vector_id, 16, 0x07);
    put_char('\n', 0x07);
}

/**
 * @brief 中断处理函数注册
 */
void intr_function_init(){
    for(int i = 0; i < IDT_DESC_CNT; i ++){
        intr_function_table[i] = general_intr_handler;
            intr_name[i] = "unknown";
    }
    intr_name[0] = "#DE Divide Error";
    intr_name[1] = "#DB Debug Exception";
    intr_name[2] = "NMI Interrupt";
    intr_name[3] = "#BP Breakpoint Exception";
    intr_name[4] = "#OF Overflow Exception";
    intr_name[5] = "#BR BOUND Range Exceeded Exception";
    intr_name[6] = "#UD Invalid Opcode Exception";
    intr_name[7] = "#NM Device Not Available Exception";
    intr_name[8] = "#DF Double Fault Exception";
    intr_name[9] = "Coprocessor Segment Overrun";
    intr_name[10] = "#TS Invalid TSS Exception";
    intr_name[11] = "#NP Segment Not Present";
    intr_name[12] = "#SS Stack Fault Exception";
    intr_name[13] = "#GP General Protection Exception";
    intr_name[14] = "#PF Page-Fault Exception";
    //  intr_name[15] 第15项是intel保留项，未使用
    intr_name[16] = "#MF x87 FPU Floating-Point Error";
    intr_name[17] = "#AC Alignment Check Exception";
    intr_name[18] = "#MC Machine-Check Exception";
    intr_name[19] = "#XF SIMD Floating-Point Exception";
}

/**
 * @brief PIC 8259A的初始化
*/
void pic_init(){
    // master
    outb(PIC_M_CTRL, 0x11); //  ICW1: 00010001b IC4=1(需要ICW4),SNGL=0(级联),LTIM=0(边沿触发)
    outb(PIC_M_DATA, 0x20); //  ICW2: 表示主片的起始中断向量号为0x20,也就是IR[0-7]
    outb(PIC_M_DATA, 0x04); //  ICW3: 00000010b IR2为接入从片的引脚
    outb(PIC_M_DATA, 0x01); //  ICW4: PM=1(8086模式), EOI=0(手动发送EOI)

    // slave
    outb(PIC_S_CTRL, 0x11); //  ICW1: 同上
    outb(PIC_S_DATA, 0x28); //  ICW2: 起始中断向量号为0x28,也就是IR[8-15] 
    outb(PIC_S_DATA, 0x02); //  ICW3: 同上
    outb(PIC_S_DATA, 0x01); //  ICW4: 同上

    // 设置中断屏蔽寄存器IMR
    //  outb(PIC_M_DATA, 0xfe); //  1111_1110b
    outb(PIC_M_DATA, 0xfe); //  1111_1011b
    outb(PIC_S_DATA, 0xff); //  1111_1111b
    
    put_str("\npic_init done\n", 0x07);
}

/**
 * @brief 中断初始化
*/
void interrupt_init(){ 
    put_str("\ninterrupt init start\n", 0x07);
    idt_init(); 
    intr_function_init();
    pic_init();

    // load idt
    uint64_t idt_operand = ((sizeof(idt) - 1) | (uint64_t)((uint32_t) idt << 16)); // 段基址 段界限
    asm volatile("lidt %0" : : "m" (idt_operand));

    put_str("\ninterrupt init done\n", 0x07);
}

/**
 * @brief 获取当前中断的状态
 */
enum intr_status intr_get_status() {
    uint32_t eflags = 0;
    asm volatile("pushfl; popl %0" : "=g" (eflags));
    return (EFLAGS_IF & eflags) ? INTR_ON : INTR_OFF;
}


void register_handler(uint8_t vector_id, void *function){
    intr_function_table[vector_id] = function;
}

/** 
 * @brief 开中断
 * @return 开中断之前的状态
 */
enum intr_status intr_enable(){
    enum intr_status old_status = intr_get_status();
    if(old_status == INTR_OFF){
        asm volatile("sti"); // 将IF位置1，开中断
    }
    return old_status;
}

/** 
 * @brief 关中断
 * @return 关中断之前的状态
 */
enum intr_status intr_disable(){
    enum intr_status old_status = intr_get_status();
    if(old_status == INTR_ON){
        asm volatile("cli" : : : "memory"); // 将IF位置9，关中断
    }
    return old_status;
}

/**
 * @brief 设置中断的状态
 * 
 * @param status 
 * @return enum intr_status 
 */
enum intr_status intr_set_status(enum intr_status status){
    return (status == INTR_ON) ? intr_enable() : intr_disable();
}
