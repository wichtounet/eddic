_F7printlnC:
push ebp
mov ebp, esp
sub esp, 4

push eax
push ebx
push ecx
push edx
push esi

mov [ebp - 4], ecx

mov esi, 0
mov eax, 4
mov ebx, 1
lea ecx, [ebp - 4]
mov edx, 1
int 80h

pop esi
pop edx
pop ecx
pop ebx
pop eax

add esp, 4
leave
ret
