_F7printlnI:
push rbp
mov rbp, rsp
push rax
push rbx
push rcx
push rdx
push rsi
push rdi
;The parameter is in r14
mov rax, r14
xor r14, r14
;If the number is negative, we print the - and then the number
or rax, rax
jge .loop
neg rax
;Print "-" 
push 1
push S2
call _F5printS
add rsp, 16
;Divide rax until there is nothing to divide
.loop:
xor rdx, rdx
mov rbx, 10
div rbx
add rdx, 48
push rdx
inc r14
or rax, rax
jz .next
jmp .loop
;Print each of the char, one by one
.next:
or r14, r14
jz .exit
dec r14
mov rax, 1
mov rdi, 1
mov rsi, rsp
mov rdx, 1
syscall
add rsp, 8
jmp .next
.exit:
call _F7println
pop rdi
pop rsi
pop rdx
pop rcx
pop rbx
pop rax
leave
ret
