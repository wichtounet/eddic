_F7printlnF:
push rbp
mov rbp, rsp
push rax
push rbx
push r14
sub rsp, 8
movq [rsp], xmm1
sub rsp, 8
movq [rsp], xmm2
cvttsd2si rbx, xmm7
cvtsi2sd xmm1, rbx
;Print the integer part
mov r14, rbx
call _F5printI
;Print the dot char
push S4
push 1
call _F5printS
add rsp, 16
;Handle negative numbers
or rbx, rbx
jge .pos
mov rbx, __float64__(-1.0)
movq xmm2, rbx
mulsd xmm7, xmm2
mulsd xmm1, xmm2
.pos:
;Remove the integer part from the floating point 
subsd xmm7, xmm1
mov rcx, __float64__(10000.0)
movq xmm2, rcx
mulsd xmm7, xmm2
cvttsd2si rbx, xmm7
mov rax, rbx
;Handle numbers with no decimal part 
or rax, rax
je .end
;Handle numbers with 0 at the beginning of the decimal part
xor r14, r14
.start:
cmp rax, 1000
jge .end
call _F5printI
imul rax, 10
jmp .start
;Print the number itself
.end:
mov r14, rbx
call _F5printI
call _F7println
movq xmm2, [rsp]
add rsp, 8
movq xmm1, [rsp]
add rsp, 8
pop r14
pop rbx
pop rax
leave
ret
