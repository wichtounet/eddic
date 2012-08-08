_F5printC:
push rbp
mov rbp, rsp
push rax
push rcx
push rdi
push rsi
push rdx

mov rax, 1
mov rdi, 1
mov rsi, r12
mov rdx, 1
syscall

pop rdx
pop rsi
pop rdi
pop rcx
pop rax
leave
ret
