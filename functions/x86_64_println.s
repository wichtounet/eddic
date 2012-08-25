_F7println:
push rbp
mov rbp, rsp

push 1
push S1
call _F5printS
add rsp, 16

leave
ret
