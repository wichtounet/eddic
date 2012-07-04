_F7printlnB:
push ebp
mov ebp, esp
push eax
push ecx
mov eax, [ebp+8] 
or eax, eax
jne .true_print
xor ecx, ecx
call _F5printI
jmp .end
.true_print:
mov ecx, 1
call _F5printI
.end:
call _F7println
pop ecx
pop eax
leave
ret
