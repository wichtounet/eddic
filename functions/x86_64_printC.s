_F5printC:
push rbp
mov rbp, rsp
sub rsp, 8

push rax
push rcx
push rdi
push rsi
push rdx

mov [rbp - 8], r14

mov rax, 1
mov rdi, 1
lea rsi, [rbp - 8]
mov rdx, 1
syscall

pop rdx
pop rsi
pop rdi
pop rcx
pop rax

add rsp, 8
leave
ret
