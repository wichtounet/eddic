_F9read_char:
push ebp
mov ebp, esp
sub esp, 4

push ebx
push ecx
push edx
push esi

mov dword [ebp - 4], 0

mov eax, 3
mov ebx, 0
lea ecx, [ebp - 4]
mov edx, 1
int 80h

mov eax, [ebp - 4]

pop esi
pop edx
pop ecx
pop ebx

add esp, 4
leave
ret
