_F7printlnC:
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

call _F7println
pop rdx
pop rsi
pop rdi
pop rcx
pop rax
leave
ret
