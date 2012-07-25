_F4freePII:
push rbp
mov rbp, rsp

push r8

;block->available = 1
lea r8, [r14 - 8]
sub r8, r15

mov qword [r8], 1

pop r8

leave
ret
