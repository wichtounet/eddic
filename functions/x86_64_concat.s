_F6concatSS:
push rbp
mov rbp, rsp
mov rbx, [rbp + 32]
mov rcx, [rbp + 16]
add rbx, rcx
;alloc the total number of bytes
push rbx
call eddi_alloc
add rsp, 8
;destination address for the movsb
mov rdi, rax
;number of bytes of the source
mov rcx, [rbp + 32]
;source address
mov rsi, [rbp + 40]
;copy the first part of the string into the destination
rep movsb
;number of bytes of the source
mov rcx, [rbp + 16]
;source address
mov rsi, [rbp + 24]
;copy the second part of the string into the destination
rep movsb
leave
ret
