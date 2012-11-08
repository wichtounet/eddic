_F5printS:
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
mov ecx, [ebp + 8]
mov edx, [ebp + 12]
int 80h

pop esi
pop edx
pop ecx
pop ebx
pop eax

leave
ret
