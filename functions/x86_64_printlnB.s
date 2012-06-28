_F7printlnB:
push rbp
mov rbp, rsp
push rax
mov rax, [rbp + 16]
or rax, rax
jne .true_print
xor r14, r14
call _F5printI
jmp .end
.true_print:
mov r14, 1
call _F5printI
.end:
call _F7println
pop rax
leave
ret
