%include "boot.inc"

SECTION loader vstart=LOADER_BASE_ADDR
    LOADER_STACK_TOP equ LOADER_BASE_ADDR
    jmp loader_start

    ;--------------- build GDT ---------------
    
    ;the 0th: no use
    ;dd: double word = 2 word = 4 byte
    GDT_BASE: dd 00000000h
              dd 00000000h
    ;the 1st: 代码段描述符
    DESC_CODE: dd DESC_CODE_LOW4
               dd DESC_CODE_HIGH4
    ;the 2nd: 数据段描述符 and 栈段描述符
    DESC_STACK_DATA: dd DESC_DATA_LOW4
                     dd DESC_DATA_HIGH4
    ;the 3rd: 显存段描述符
    DESC_VIDEO: dd DESC_VIDEO_LOW4
                dd DESC_VIDEO_HIGH4
    
    GDT_SIZE  equ $ - GDT_BASE
    GDT_LIMIT equ GDT_SIZE - 1
    times 20 dq 0 ;预留20个8B的段描述符空位

    SELECTOR_CODE  equ (0x0001 << 3) + TI_GDT + RPL0 
    SELECTOR_DATA  equ (0x0002 << 3) + TI_GDT + RPL0
    SELECTOR_VIDEO equ (0x0003 << 3) + TI_GDT + RPL0 

    gdt_ptr dw GDT_LIMIT
            dd GDT_BASE
    loadermsg db '2 loader in real.'
    ; loadermsglen dw $ - loadermsg
    
    ;--------------- loader ---------------

loader_start:
; INT 10h / AH = 13h - write string.
; input:
; AL = write mode:
;     bit 0: update cursor after writing;
;     bit 1: string contains attributes.
; BH = page number.
; BL = attribute if string contains only characters (bit 1 of AL is zero).
; CX = number of characters in string (attributes are not counted).
; DL,DH = column, row at which to start writing.
; ES:BP points to string to be printed.
    mov sp, LOADER_BASE_ADDR
    mov ax, 1301h
    mov bx, 001fh
    mov cx, 17
    mov dh, 18h ;row
    mov dl, 00h ;col
    mov bp, loadermsg
    int 10h 

    ;---------------  enter protect mode --------------- 
    ;1 open A20
    in al, 92h
    or al, 0000_0010b
    out 92h, al

    ;2 load GDT
    lgdt [gdt_ptr]

    ;3 cr0 pe=1
    mov eax, cr0
    or eax, 0000_0001h
    mov cr0, eax 

    jmp dword SELECTOR_CODE:p_mode_start ;jmp远跳转, 清空流水线

[bits 32]
p_mode_start:
    mov ax, SELECTOR_DATA
    mov ds, ax 
    mov es, ax 
    mov ss, ax 
    mov esp, LOADER_STACK_TOP
    mov ax, SELECTOR_VIDEO
    mov gs, ax 

    mov byte [gs:0x10], 'P'
    mov byte [gs:0x11], 0xA4

    ;---------------  load kernel --------------- 
    mov eax, KERNEL_START_SECTOR
    mov ebx, KERNEL_COM_BASE_ADDR
    mov ecx, KERNEL_SECTOR_CNT

    call ReadDisk_32

    ;---------------  启用分页机制 --------------- 
    call setup_page

    sgdt [gdt_ptr] ;将gdt倒回gdt_ptr

    ;显存段描述符 段基址+3GB(移到操作系统的内存空间)
    mov ebx, [gdt_ptr + 2]
    or dword [ebx + 24 + 4], 0xc000_0000

    ;将GDT基址+3Gb(移到操作系统的内存空间)
    add dword [gdt_ptr + 2], 0xc000_0000 
    
    ;将栈指针+3GB
    add esp, 0xc000_0000

    ;cr3 = 页目录表地址
    mov eax, PAGE_DIR_TABLE_POS
    mov cr3, eax 

    ;打开cr0的pg位
    mov eax, cr0
    or eax, 0x8000_0000
    mov cr0, eax 

    lgdt [gdt_ptr]

    ;---------------  enter kernel --------------- 

    mov byte [gs:0x20], 'V' 
    mov byte [gs:0x21], 0xA4

    jmp SELECTOR_CODE:enter_kernel ;刷新流水线

enter_kernel:
    call kernel_init
    mov esp, 0xc009_f000 
    jmp KERNEL_ENTRY_POINT

    ; jmp $ 

    ;---------------  将kernel.com中的segment拷贝到编译的地址 --------------- 
kernel_init:
    ;clear 
    xor eax, eax 
    xor ebx, ebx
    xor ecx, ecx 
    xor edx, edx 

    mov dx, [KERNEL_COM_BASE_ADDR + 42] ;e_phentsize, program header的大小
    mov ebx, [KERNEL_COM_BASE_ADDR + 28] ;e_phoff, 1st program header在文件中的偏移量
    
    add ebx, KERNEL_COM_BASE_ADDR ;ebx: file base addr + e_phoff = program header base addr
    mov cx, [KERNEL_COM_BASE_ADDR + 44] ;e_phnum, program header的数量
.each_segment:
    cmp byte [ebx + 0], PT_NULL
    je .prepare_next

    ;处理非空段: copy
    push dword [ebx + 16] ;p_filesz (size)
    mov eax, [ebx + 4] ;p_offset
    add eax, KERNEL_COM_BASE_ADDR 
    push eax ;段的物理地址 (src)
    push dword [ebx + 8] ;p_vaddr 段的虚拟地址(dst)
    call mem_cpy
    add esp, 12 ;4B * 3
.prepare_next: 
    add ebx, edx ;next program header
    loop .each_segment
    ret 


mem_cpy:
    ;params: (dst, src, len)
    cld ;DF=0, 表示地址递增
    push ebp 
    mov ebp, esp
    push ecx 
    mov edi, [ebp + 8]  ;dst
    mov esi ,[ebp + 12] ;src
    mov ecx, [ebp + 16] ;size
    
    ;为了防止.note.gnu.property要加载到超过1MB的部分
    push eax
    push ebx
    mov ebx, 0
    mov eax, edi
    add eax, ecx
.cmp1: cmp eax, 0xc000_0000
    jl .cmp2
    inc ebx
.cmp2: cmp eax, 0xc010_0000
    jg .cmp3
    inc ebx
.cmp3: cmp eax, 0x0010_0000
    jg .check
    inc ebx
.check:
    cmp ebx, 0x2
    jl .mem_cpy_end    
    rep movsb ;move byte

.mem_cpy_end:
    pop ebx 
    pop eax 
    pop ecx 
    pop ebp 
    ret


    ;--------------- - 页目录表 and 页表 --------------- -
setup_page:
    ;清空页目录表内存 4KB = 4096B
    mov ecx, 4096
    mov esi, 0
    clear_page_dir:
        mov byte [PAGE_DIR_TABLE_POS + esi], 0
        inc esi 
        loop clear_page_dir

    ;创建页目录项Page Directory Entry
    create_pde:
        mov eax, PAGE_DIR_TABLE_POS
        add eax, 0x1000 
        mov ebx, eax ;ebx: 1st page table base pos

        or eax, PG_US_U | PG_RW_W | PG_P ;eax: 第0个页表物理页地址31-12位, US=User,RW=W,P=1
        mov [PAGE_DIR_TABLE_POS + 0x0], eax ;第0个目录项对应的页表: 0~0x3fffff(4MB)
        mov [PAGE_DIR_TABLE_POS + 0xc00], eax ;第768个目录项对应的页表: 0xc0000000~0xc3fffff, 即3GB～3GB+4MB，操作系统所在位置

        sub eax, 0x1000 ;eax: 页目录表自身的物理页地址
        mov [PAGE_DIR_TABLE_POS + 4092], eax ;第1023个目录项对应的页表: 页目录表自身

        ;创建第0个目录项对应的页表(管理内存 0~0x3fffff)-页表项Page Table Entry
        ;使用1MB内存(0~0xfffff), 256页
        mov ecx, 256
        mov esi, 0
        mov edx, PG_US_U | PG_RW_W | PG_P
        .create_pte:
            mov [ebx + esi * 4], edx
            add edx, 4096 ;4KB
            inc esi 
            loop .create_pte
        
        ;创建操作系统剩余目录项
        ;操作系统（内存空间：3GB~4GB), 第768～1022个目录项
        mov eax, PAGE_DIR_TABLE_POS
        add eax, 0x2000 ;eax: 2nd page table
        or eax, PG_US_U | PG_RW_W | PG_P
        mov ebx, PAGE_DIR_TABLE_POS
        mov ecx, 254 ;1022-769+1
        mov esi, 769
        .create_kernel_pde:
            mov [ebx + esi * 4], eax 
            inc esi 
            add eax, 0x1000
            loop .create_kernel_pde
        
        ret



ReadDisk_32:	   
;params: eax - start sector; ebx - load address; ecx - number of sectors
    ;copy
    mov esi, eax
    mov di, cx
;读写硬盘:
;第1步：设置要读取的扇区数
    mov dx, 0x1f2
    mov al, cl
    out dx, al ;读取的扇区数

    mov eax, esi	;恢复ax

;第2步：将LBA地址存入0x1f3 ~ 0x1f6

    ;LBA地址7~0位写入端口0x1f3
    mov dx, 0x1f3                       
    out dx, al                          

    ;LBA地址15~8位写入端口0x1f4
    mov cl, 8
    shr eax, cl
    mov dx, 0x1f4
    out dx, al

    ;LBA地址23~16位写入端口0x1f5
    shr eax, cl
    mov dx, 0x1f5
    out dx, al

    shr eax, cl
    and al, 0x0f	;lba第24~27位
    or al, 0xe0	;设置7～4位为1110,表示lba模式
    mov dx, 0x1f6
    out dx, al

;第3步：向0x1f7端口写入读命令，0x20 
    mov dx, 0x1f7
    mov al, 0x20                        
    out dx, al

;;;;;;; 至此,硬盘控制器便从指定的lba地址(eax)处,读出连续的cx个扇区,下面检查硬盘状态,不忙就能把这cx个扇区的数据读出来

;第4步：检测硬盘状态
.not_ready:		   ;测试0x1f7端口(status寄存器)的的BSY位
    ;同一端口,写时表示写入命令字,读时表示读入硬盘状态
    nop
    in al, dx
    and al, 0x88	   ;第4位为1表示硬盘控制器已准备好数据传输,第7位为1表示硬盘忙
    cmp al, 0x08
    jnz .not_ready	   ;若未准备好,继续等。

;第5步：从0x1f0端口读数据
    mov ax, di
    mov dx, 256	   ;di为要读取的扇区数,一个扇区有512字节,每次读入一个字,共需di*512/2次,所以di*256
    mul dx
    mov cx, ax	   
    mov dx, 0x1f0
.go_on_read:
    in ax, dx		
    mov [ebx], ax
    add ebx, 2
            ; 由于在实模式下偏移地址为16位,所以用bx只会访问到0~FFFFh的偏移。
            ; loader的栈指针为0x900,bx为指向的数据输出缓冲区,且为16位，
            ; 超过0xffff后,bx部分会从0开始,所以当要读取的扇区数过大,待写入的地址超过bx的范围时，
            ; 从硬盘上读出的数据会把0x0000~0xffff的覆盖，
            ; 造成栈被破坏,所以ret返回时,返回地址被破坏了,已经不是之前正确的地址,
            ; 故程序出会错,不知道会跑到哪里去。
            ; 所以改为ebx代替bx指向缓冲区,这样生成的机器码前面会有0x66和0x67来反转。
            ; 0X66用于反转默认的操作数大小! 0X67用于反转默认的寻址方式.
            ; cpu处于16位模式时,会理所当然的认为操作数和寻址都是16位,处于32位模式时,
            ; 也会认为要执行的指令是32位.
            ; 当我们在其中任意模式下用了另外模式的寻址方式或操作数大小(姑且认为16位模式用16位字节操作数，
            ; 32位模式下用32字节的操作数)时,编译器会在指令前帮我们加上0x66或0x67，
            ; 临时改变当前cpu模式到另外的模式下.
            ; 假设当前运行在16位模式,遇到0X66时,操作数大小变为32位.
            ; 假设当前运行在32位模式,遇到0X66时,操作数大小变为16位.
            ; 假设当前运行在16位模式,遇到0X67时,寻址方式变为32位寻址
            ; 假设当前运行在32位模式,遇到0X67时,寻址方式变为16位寻址.

    loop .go_on_read
    ret
