[bits 32]

section .text
global switch_to
switch_to:
;(now, next) 两个struct pcb *
    push esi
    push edi
    push ebx
    push ebp
    ;stack(上到下): next, now, return address, esi, edi, ebx, ebp

    ;保存上下文
    mov eax, [esp + 20] ; now
    mov [eax], esp ;将esp保存到PCB的self_kstack中（顶端）

    ;恢复上下文
    mov eax, [esp + 24]	; next
    mov esp, [eax]		 

    pop ebp
    pop ebx
    pop edi
    pop esi
    ret	
