_F7printlnS:
push rbp
mov rbp, rsp

push rax
push rcx
push rdi
push rsi
push rdx
push r11

mov rax, 1
mov rdi, 1
mov rsi, [rbp + 16]
mov rdx, [rbp + 24]
syscall

call _F7println

pop r11
pop rdx
pop rsi
pop rdi
pop rcx
pop rax

leave
ret
