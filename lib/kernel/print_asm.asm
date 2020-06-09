TI_GDT equ  0
RPL0  equ   0
SELECTOR_VIDEO equ (0x0003 << 3) + TI_GDT + RPL0

[bits 32]
section .text

global put_char
global put_char_pos
global get_char_pos
global put_str
global set_cursor
global get_cursor_pos

extern put_int

put_str: ;(str, attr)
    pushad 
    xor ecx, ecx 
    mov ebx, [esp + 36] ;第一个参数: 字符串地址
    mov eax, [esp + 40] ;第二个参数: 字符属性
.str_loop:
    mov cl, [ebx] ;al: 要输出的字符
    cmp cl, 0
    jz .str_over
    push eax 
    push ecx 
    call put_char
    add esp, 8
    inc ebx 
    jmp .str_loop
.str_over:
    popad
    ret

put_char: ;(char, attr)
    pushad  ;save 32bit registers(压入4*8=32B)
    mov ax, SELECTOR_VIDEO
    mov gs, ax  ;gs = 显存段选择子

    ;----- 获取光标位置（下一个可打印字符的位置） -----
    ;高8位
    mov dx, 0x03d4  ;CRT controller寄存器组的Address Register端口地址
    mov al, 0x0e    ;curosr location high register的索引为0x0e
    out dx, al      ;对应的data register改变
    mov dx, 0x03d5  ;CRT controller寄存器组的Data Register端口地址
    in al, dx       ;读出光标位置的高8位
    mov ah, al
    ;低8位
    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5
    in al, dx 
    ;将光标存入bx 
    mov bx, ax 
    ;打印字符
    mov ecx, [esp + 36] ;参数c（pushad 32B, 返回地址4B）
    cmp cl, 0xd ;carriage return(CR)回车符
    jz .is_CR
    cmp cl, 0xa ;line_feed(LF)换行符
    jz .is_LF
    cmp cl, 0x8 ;backspace(BS)回退符
    jz .is_BS
    jmp .put_other

.is_BS: ;回退
    dec bx ;光标回退
    shl bx, 1   ;光标位置*2 = 显存位置
    mov byte [gs:bx], 0x20  ;在上一个字符位置填空格
    inc bx
    mov byte [gs:bx], 0x07
    shr bx, 1
    jmp .set_cursor

.put_other:
    shl bx, 1
    mov [gs:bx], cl
    inc bx 
    mov eax, [esp + 40] ;第二个参数，字符属性
    mov byte [gs:bx], al
    shr bx, 1
    inc bx 
    cmp bx, 2000    ;若满屏，换行即可 is_LF
    jl .set_cursor

.is_LF: ;换行符: 切换到下一行 \n

.is_CR: ;回车符: 光标撤回行首 \r
;按下Enter:回车+换行
    ;处理回车
    xor dx, dx ;被除数高16位
    mov ax, bx ;被除数低16位
    mov si, 80 ;除数
    div si
    sub bx, dx ;bx = bx - bx % 80, 即当前行首坐标

.is_CR_end:
    ;处理换行
    add bx, 80  ;换到下一行
    cmp bx, 2000
.is_LF_end:
    jl .set_cursor

.roll_screen:
    ;将1～24行拷贝到0~23行
    cld 
    mov ecx, 960 ;24*80/2
    mov esi, 0xb80a0 ;第1行行首
    mov edi, 0xb8000 ;第0行行首
    rep movsd 
    ;第24行填空格
    mov ebx, 3840 ;1920*2
    mov ecx, 80
.cls:
    mov word [gs:ebx], 0x0720 ;空格，黑底白字
    add ebx, 2
    loop .cls 
    mov bx, 1920 ;光标为第24行首

.set_cursor: ;将光标设为bx的值
    ;高8位
    mov dx, 0x03d4
    mov al, 0x0e
    out dx, al
    mov dx, 0x03d5
    mov al, bh
    out dx, al 
    ;低8位
    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5
    mov al, bl
    out dx, al

.put_char_done:
    popad
    ret

put_char_pos: ;(char, attr, pos)
    pushad  ;save 32bit registers(压入4*8=32B)
    mov ax, SELECTOR_VIDEO
    mov gs, ax  ;gs = 显存段选择子

    ;将位置存入bx 
    mov eax, [esp + 44] ;第三个参数, pos
    mov bx, ax

    ;打印字符
    mov ecx, [esp + 36] ;参数c（pushad 32B, 返回地址4B）

    shl bx, 1
    mov [gs:bx], cl
    inc bx 
    mov eax, [esp + 40] ;第二个参数，字符属性
    mov byte [gs:bx], al
    shr bx, 1
    inc bx 
 
    popad
    ret

set_cursor: ;(pos)
    pushad

    mov bx, [esp + 36] ;第一个参数
    ;高8位
    mov dx, 0x03d4
    mov al, 0x0e
    out dx, al
    mov dx, 0x03d5
    mov al, bh
    out dx, al
    ;低8位
    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5
    mov al, bl
    out dx, al
    
    popad
    ret 

get_char_pos: ;(pos)
    ; pushad  ;save 32bit registers(压入4*8=32B)
    push ecx 
    push edx 
    push ebx 
    push esp 
    push ebp
    push esi 
    push edi 

    mov ax, SELECTOR_VIDEO
    mov gs, ax  ;gs = 显存段选择子

    ;将位置存入bx 
    mov eax, [esp + 32] ;第1个参数, pos
    mov bx, ax

    ;打印字符
    ; mov ecx, [esp + 36] ;参数c（pushad 32B, 返回地址4B）

    shl bx, 1
    mov byte ah, [gs:bx] ;字符
    inc bx 
    mov byte al, [gs:bx] ;字符属性
    shr bx, 1
    inc bx 
    
    pop edi 
    pop esi 
    pop ebp
    pop esp 
    pop ebx 
    pop edx 
    pop ecx 
    ; popad
    ret

get_cursor_pos:
    push ecx 
    push edx 
    push ebx 
    push esp 
    push ebp
    push esi 
    push edi 

    mov ax, SELECTOR_VIDEO
    mov gs, ax  ;gs = 显存段选择子

    ;----- 获取光标位置（下一个可打印字符的位置） -----
    ;高8位
    mov dx, 0x03d4  ;CRT controller寄存器组的Address Register端口地址
    mov al, 0x0e    ;curosr location high register的索引为0x0e
    out dx, al      ;对应的data register改变
    mov dx, 0x03d5  ;CRT controller寄存器组的Data Register端口地址
    in al, dx       ;读出光标位置的高8位
    mov ah, al
    ;低8位
    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5
    in al, dx 
    
    pop edi 
    pop esi 
    pop ebp
    pop esp 
    pop ebx 
    pop edx 
    pop ecx 
    ret


