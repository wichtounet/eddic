_F7println:
push rbp
mov rbp, rsp
push S1
push 1
call _F5printS
add rsp, 16
leave
ret
