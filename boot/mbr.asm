%include "boot.inc"

SECTION MBR vstart=0x7c00
;BIOS use 'jmp 0:0x7c00' to jump to MBR, so cs = 0 now.
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov sp, 0x7c00
    mov ax, 0xb800
    mov gs, ax

ClearScreen:
; INT 10h / AH = 06h - scroll up window.
; input:
; AL = number of lines by which to scroll (00h = clear entire window).
; BH = attribute used to write blank lines at bottom of window.
; CH, CL = row, column of window's upper left corner.
; DH, DL = row, column of window's lower right corner.
    mov ax, 0600h
    mov bx, 0700h
    mov cx, 0000h ;(0, 0)
    mov dx, 184fh ;(24, 79)
    int 10h

ShowMessage:
    mov byte [gs:0x00],'1'
    mov byte [gs:0x01],0x07

    mov byte [gs:0x02],' '
    mov byte [gs:0x03],0x07

    mov byte [gs:0x04],'M'
    mov byte [gs:0x05],0x07

    mov byte [gs:0x06],'B'
    mov byte [gs:0x07],0x07

    mov byte [gs:0x08],'R'
    mov byte [gs:0x09],0x07
        
; LoadLoader:
;     mov bx, LOADER_BASE_ADDR
;     mov ah, 02h
;     mov al, 02h ; 扇区数
;     mov dx, 0000h
;     mov ch, 00h 
;     mov cl, LOADER_START_SECTOR
;     int 13h
;     jmp LOADER_BASE_ADDR

    
    mov eax, LOADER_START_SECTOR
    mov bx, LOADER_BASE_ADDR
    mov cx, 2
    call ReadDisk_16	 
  
   jmp LOADER_BASE_ADDR
       

ReadDisk_16:
;params: eax - start sector; bx - load address; cx - number of sectors
    ;copy 
    mov esi,eax 
    mov di,cx	
;读写硬盘:
;第1步：设置要读取的扇区数
    mov dx,0x1f2
    mov al,cl
    out dx,al        ;读取的扇区数

    mov eax,esi	   ;恢复ax

;第2步：将LBA地址存入0x1f3 ~ 0x1f6

    ;LBA地址7~0位写入端口0x1f3
    mov dx,0x1f3                       
    out dx,al                          

    ;LBA地址15~8位写入端口0x1f4
    mov cl,8
    shr eax,cl
    mov dx,0x1f4
    out dx,al

    ;LBA地址23~16位写入端口0x1f5
    shr eax,cl
    mov dx,0x1f5
    out dx,al

    shr eax,cl
    and al,0x0f	   ;lba第24~27位
    or al,0xe0	   ;设置7～4位为1110,表示lba模式
    mov dx,0x1f6
    out dx,al

;第3步：向0x1f7端口写入读命令，0x20 
    mov dx,0x1f7
    mov al,0x20                        
    out dx,al

;第4步：检测硬盘状态
.not_ready:
    ;同一端口，写时表示写入命令字，读时表示读入硬盘状态
    nop
    in al,dx
    and al,0x88	   ;第4位为1表示硬盘控制器已准备好数据传输，第7位为1表示硬盘忙
    cmp al,0x08
    jnz .not_ready	   ;若未准备好，继续等。

;第5步：从0x1f0端口读数据
    mov ax, di
    mov dx, 256
    mul dx
    mov cx, ax	   ;di为要读取的扇区数，一个扇区有512字节，每次读入一个字，
                    ;共需di*512/2次，所以di*256
    mov dx, 0x1f0
.go_on_read:
    in ax,dx
    mov [bx],ax
    add bx,2		  
    loop .go_on_read
    ret


MBREnd:
    times 510-($-$$) db 0
    db 0x55, 0xaa