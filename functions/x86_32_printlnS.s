_F7printlnS:
push ebp
mov ebp, esp
push eax
push ebx
push ecx
push edx
push esi
mov esi, 0
mov eax, 4
mov ebx, 1
mov ecx, [ebp + 12]
mov edx, [ebp + 8]
int 80h
call _F7println
pop esi
pop edx
pop ecx
pop ebx
pop eax
leave
ret
