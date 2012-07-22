_F5printB:
push ebp
mov ebp, esp
push ecx
or ecx, ecx
jne .true_print
xor ecx, ecx
call _F5printI
jmp .end
.true_print:
mov ecx, 1
call _F5printI
.end:
pop ecx
leave
ret
