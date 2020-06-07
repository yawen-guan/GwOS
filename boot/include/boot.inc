;----------- loader, kernel ------------
LOADER_BASE_ADDR equ 0x900
LOADER_START_SECTOR equ 0x1

KERNEL_COM_BASE_ADDR equ 0x7_0000
KERNEL_START_SECTOR equ 0x8
KERNEL_ENTRY_POINT equ 0xc000_1500
KERNEL_SECTOR_CNT equ 50

;----------- Page Table ------------

PAGE_DIR_TABLE_POS equ 0x100000

;----------- GBT -----------

;代码段描述符: 段基址 00000000h, 段界限 FFFFFh (单位为4KB, 即4GB)
;数据段描述符: 段基址 00000000h, 段界限 FFFFFh (同上)
;显存段描述符: 段基址 000B8000h, 段界限 00007h (BFFFF-B8000)/4KB = 7h

DESC_G_4K equ      1_0_0_0_0000_0_00_0_0000_00000000b ;G=1 4K
DESC_D_32 equ        1_0_0_0000_0_00_0_0000_00000000b ;D=1 32bit
DESC_L    equ          0_0_0000_0_00_0_0000_00000000b ;L=0 32bit
DESC_AVL  equ            0_0000_0_00_0_0000_00000000b ;AVL=0
DESC_LIMIT_HIGH_CODE equ   1111_0_00_0_0000_00000000b
DESC_LIMIT_HIGH_DATA equ   1111_0_00_0_0000_00000000b
DESC_LIMIT_HIGH_VIDEO equ  0000_0_00_0_0000_00000000b
DESC_P    equ                   1_00_0_0000_00000000b ;P=1 present
DESC_DPL_0 equ                    00_0_0000_00000000b ;特权级 0
DESC_DPL_1 equ                    01_0_0000_00000000b ;特权级 1
DESC_DPL_2 equ                    10_0_0000_00000000b ;特权级 2
DESC_DPL_3 equ                    11_0_0000_00000000b ;特权级 3
DESC_S_CODE equ                      1_0000_00000000b ;s=1 非系统段
DESC_S_DATA equ                      1_0000_00000000b 
DESC_S_SYS  equ                      0_0000_00000000b ;s=0 系统段
DESC_TYPE_CODE  equ                    1000_00000000b ;只执行代码段
DESC_TYPE_DATA  equ                    0010_00000000b ;只读，向下扩展的数据段

;High 4B
DESC_CODE_HIGH4  equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_HIGH_CODE  + DESC_P + DESC_DPL_0 + DESC_S_CODE + DESC_TYPE_CODE + 0x00
DESC_DATA_HIGH4  equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_HIGH_DATA  + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x00
DESC_VIDEO_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_HIGH_VIDEO + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x0B

;Low 4B
DESC_CODE_LOW4  equ 0000FFFFh
DESC_DATA_LOW4  equ 0000FFFFh
DESC_VIDEO_LOW4 equ 80000007h

;----------- Selector ------------
RPL0 equ 00b ;请求特权级 0
RPL1 equ 01b 
RPL2 equ 10b 
RPL3 equ 11b 
TI_GDT equ 000b ;选择子在GDT中 
TI_LDT equ 001b ;选择子在LDT中

;----------- Page Table ----------
PG_P equ        1b ;P=1,该表存在物理内存中
PG_RW_R equ   0_0b ;RW=0, 可读不可写
PG_RW_W equ   1_0b ;RW=1, 可读可写
PG_US_S equ 0_0_0b ;US=0, Supervisor, 特权级012才可访问该页
PG_US_U equ 1_0_0b ;US=1, User, 特权级0123均可访问该页

;----------- Program Type(ELF Files) -----------
PT_NULL equ 0 ;e_ident=0, 表示不可识别类型