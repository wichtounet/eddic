_F7println:
push ebp
mov ebp, esp

push 1
push S1
call _F5printS
add esp, 8

leave
ret
