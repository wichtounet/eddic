_F4init:
push rbp
mov rbp, rsp

; get start of system brk
mov rax, 12
xor rdi, rdi
syscall

mov [V_mem_start], rax
mov [V_mem_last], rax

leave
ret
