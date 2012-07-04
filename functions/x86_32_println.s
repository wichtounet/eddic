_F7println:
push ebp
mov ebp, esp
push S1
push 1
call _F5printS
add esp, 8
leave
ret
