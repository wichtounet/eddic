_F4init:
push ebp
mov ebp, esp

; get start of system brk
xor ebx, ebx
mov eax, 45
int 80h

mov [V_mem_start], eax
mov [V_mem_last], eax

leave
ret
