_F5printB:
push rbp
mov rbp, rsp
push r14
or r14, r14
jne .true_print
xor r14, r14
call _F5printI
jmp .end
.true_print:
mov r14, 1
call _F5printI
.end:
pop r14
leave
ret
