_F7printlnS:
push rbp
mov rbp, rsp
push rax
push rcx
push rdi
push rsi
push rdx
mov rax, 1
mov rdi, 1
mov rsi, [rbp + 24]
mov rdx, [rbp + 16]
syscall
call _F7println
pop rdx
pop rsi
pop rdi
pop rcx
pop rax
leave
ret
