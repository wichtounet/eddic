_F4freePII:
push rbp
mov rbp, rsp

;block->available = 1
mov qword [r14 - 16], 1

leave
ret
