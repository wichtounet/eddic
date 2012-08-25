_F9read_char:
push rbp
mov rbp, rsp
sub rsp, 8

push rcx
push rdi
push rsi
push rdx
push r11

mov qword [rbp - 8], 0

mov rax, 0
mov rdi, 0
lea rsi, [rbp - 8]
mov rdx, 1
syscall

mov rax, [rbp - 8]

pop r11
pop rdx
pop rsi
pop rdi
pop rcx

add rsp, 8
leave
ret
